#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

// For gps
#define GPS_BAUD 4800
#define GPSRATE (F_CPU / 16 / GPS_BAUD - 1)

void gps_init(void) {
	DDRD  &= 0b11111011;  // set pin 2 to 0 (receive)
	DDRD  |= 0b00001000;  // set pin 3 to 1 (send)
	PORTD &= 0b11110111;  // don't ask for data yet
}


uint8_t gps_getchar(void) {
	PORTD |= 0b00001000;  // send is high to receive data
	
	uint8_t val = 0;
	
	//Wait for start bit (zero)
	while(PIND & 0b00000100); 
  
	if(!(PIND & 0b00000100)) {
		//Read in the middle of the bit
		_delay_us(GPSRATE >> 1);
		
		//Read Characters
		for (int offset = 0; offset < 8; offset++) {
			_delay_us(GPSRATE);
			val |= ((PIND & 0b00000100) >> 2) << offset;
		}
		
		//wait for stop bit + extra
		_delay_us(GPSRATE); 
		_delay_us(GPSRATE);
	}
	
	PORTD &= 0b11110111;  // stop asking for data
	
	return val;
}
char* gps_getstring(char* outstring) {	
	// Wait for start
	while(gps_getchar() != '$');
	
	// Remember position
	uint8_t i, c;
	
	outstring[0] = '$';
	for(i = 1; (c = gps_getchar()) != 13 && i < 127; ++i) 
		outstring[i] = c;
	
	outstring[i] = 0;
	return outstring;
}
