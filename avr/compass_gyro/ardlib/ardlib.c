#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ardlib.h"
#include "i2c.h"

// Servo tuning
#define ADD_AMOUNT 1000
#define SCALE_AMOUNT 1000

// For serial port
#define SER_BAUD 9600
#define SERRATE (F_CPU / 16 / SER_BAUD - 1)

// For gps
#define GPS_BAUD 4800
#define GPSRATE (F_CPU / 16 / GPS_BAUD - 1)

// Set and clear bit defines
#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define SCP_CSB     0
#define SCP_MISO    1
#define SCP_MOSI    2
#define SCP_SCK		3

// Prototypes
static int uart_putchar(char c, FILE *stream);
uint8_t spi_comm(uint8_t outgoing_byte);

// Set up file to be used as redirected stdout
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

// Grab a character from the serial port
uint8_t uart_getchar(void)
{
    while(!(UCSR0A & (1<<RXC0)));

    return(UDR0);
}

// Write a character to the serial port
static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

// Sets up serial communication and allows printf to be mapped to serial port
void serial_init (void)
{
    DDRD &= 0b11111100; // clear bits
    DDRD |= 0b00000010; // PORTD (RX on PD0)

    //USART Baud rate: 9600
    UBRR0H = ((SERRATE) >> 8);
    UBRR0L = SERRATE;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    stdout = &mystdout; //Required for printf init
}

// Sets up adc for inputting values
void adc_init (void)
{
    DDRC = 0b11000000;

	ADCSRA|=(1<<ADEN); //|(1<<ADATE);//enable adc & auto trigger
  	ADCSRA|=(1<<ADPS1)|(1<<ADPS0);//set prescaler to 8 
}

void gps_init (void)
{
	DDRD &= 0b11110011;  // clear other bits
	DDRD |= 0b00001000;  // recv = pin 2, send = pin 3
	
	PORTD |= 0b00001000;   // send is high to receive data
}

uint8_t gps_getchar (void)
{
	uint8_t val = 0;
	
	//Wait for start bit (zero)
	while(PIND & 0b00000100); 
  
	if(!(PIND & 0b00000100))
	{
		//Read in the middle of the bit
		_delay_us(GPSRATE >> 1);
		
		//Read Characters
		for (int offset = 0; offset < 8; offset++) 
		{
			_delay_us(GPSRATE);
			val |= ((PIND & 0b00000100) >> 2) << offset;
		}
		
		//wait for stop bit + extra
		_delay_us(GPSRATE); 
		_delay_us(GPSRATE);
	}
	
	return val;
}

void gps_getstring(char *outstring)
{
	// Wait for start
	while(gps_getchar() != '$');
	
	// Remember position
	int i = 1;
	uint8_t c;
	
	outstring[0] = '$';
	
	while((c = gps_getchar()) != 13 && i < 127)
	{
		outstring[i] = c;
		
		i++;
	}
	
	outstring[i] = 0;
}

uint8_t adc_read (uint8_t which)
{
	ADMUX = which & 0x7;
	ADMUX |= (1<<REFS0) | (1<<REFS1);
	ADCSRA |= (1<<ADSC);

	while((ADCSRA & 0x40) !=0){};

	return (ADC >> 2);
}

void pwm_init (void)
{
	// Set output
	DDRB |= 0b00000010;
	
	ICR1H = (20000 >> 8) & 0xFF;   // Set top
	ICR1L = (20000) & 0xFF;   // Set top
	
	// Set the timer registers
	TCCR1A = 0b00100000; // pulse accurate waveform pwm generation
	TCCR1B = 0b00010010; // prescaler to 8, so we have top = 20,000
}

void pwm_set (uint8_t value)
{
	uint16_t servo = (uint16_t)((double)SCALE_AMOUNT * (double)value / (double)255) + ADD_AMOUNT;
	
	OCR1A = servo;
}

void spi_init (void)
{
    //1 = output, 0 = input
    DDRB &= 0b11110000;    
    DDRB |= (1<<PC0)|(1<<PC2)|(1<<PC3); 	//PC 0=CSB, 1=MISO, 2=MOSI, 3=SCK
    
    sbi(PORTB, SCP_CSB); //Deselect SCP1000
}

//Read 8-bit register
uint8_t read_register(uint8_t register_name)
{
    uint8_t in_byte;
    
    register_name <<= 2;
    register_name &= 0b11111100; //Read command

    cbi(PORTB, SCP_CSB); //Select SPI Device

    in_byte = spi_comm(register_name); //Write byte to device
    //in_byte is nothing, we need to clock in another 8 bits
    in_byte = spi_comm(0x00); //Send nothing, but we should get back the register value

    sbi(PORTB, SCP_CSB);
    
    return(in_byte);
}

//Read 16-bit register
uint16_t read_register16(uint8_t register_name)
{
    uint16_t in_byte;

    register_name <<= 2;
    register_name &= 0b11111100; //Read command

    cbi(PORTB, SCP_CSB); //Select SPI Device

    in_byte = spi_comm(register_name); //Write byte to device
    //in_byte is nothing, we need to clock in another 8 bits
    in_byte = spi_comm(0x00); //Send nothing, but we should get back the register value
    in_byte <<= 8;
    in_byte |= spi_comm(0x00); //Send nothing, but we should get back the register value

    sbi(PORTB, SCP_CSB);
    
    return(in_byte);
}

//Sends a write command to SCP1000
void write_register(uint8_t register_name, uint8_t register_value)
{
    uint8_t in_byte;
    
    register_name <<= 2;
    register_name |= 0b00000010; //Write command

    cbi(PORTB, SCP_CSB); //Select SPI device

    in_byte = spi_comm(register_name); //Send register location
    in_byte = spi_comm(register_value); //Send value to record into register
    
    sbi(PORTB, SCP_CSB);
    
    //Return nothing
}

//Basic SPI send and receive
uint8_t spi_comm(uint8_t outgoing_byte)
{
    uint8_t incoming_byte, x;

    incoming_byte = 0;
    
    for(x = 8 ; x > 0 ; x--)
    {
        cbi(PORTB, SCP_SCK); //Toggle the SPI clock

        //Put bit on SPI data bus    
        if(outgoing_byte & (1 << (x-1)))
            sbi(PORTB, SCP_MOSI);
        else
            cbi(PORTB, SCP_MOSI);

        sbi(PORTB, SCP_SCK);

        //Read bit on SPI data bus
        incoming_byte <<= 1;
        if ( (PINB & (1 << SCP_MISO)) ) incoming_byte |= 1;
    }
    
    return(incoming_byte);
}

void spi_get_values(uint16_t *temperature, uint32_t *pressure)
{
	int32_t temp_data;

    //Wait for new data
    while(1)
    {
    	temp_data = read_register(0x07); //Check the status register for bit 6 to go high
    	
    	if (temp_data & 32) break;
    }

    *temperature = read_register16(0x21) & 0b0011111111111111; //Read the temperature data

    temp_data = read_register(0x1F) & 0x7; //Read MSB pressure data - 3 lower bits
    uint32_t tPres = read_register16(0x20);

    *pressure = (temp_data << 16) | tPres;
}

