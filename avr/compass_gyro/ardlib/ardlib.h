#ifndef __ARDLIB_H
#define __ARDLIB_H

// For getting ADC pins (uses all 6 analog inputs)
void adc_init(void);
uint8_t adc_read (uint8_t which);

// For using serial (uses io pins 0, 1)
void serial_init(void);
uint8_t uart_getchar(void);

// For using software serial for 402A GPS (uses io pins 2, 3, 4)
void gps_init(void);
uint8_t gps_getchar(void);
void gps_getstring(char *outstring);

// For using i2c to get compass headings (uses io pins 5, 6)
void compass_init(void);
uint32_t compass_get_heading (void);

// For use with servos or other pwm things
void pwm_init (void);
void pwm_set (uint8_t value);

// For use with pressure/temperature sensor
void spi_init(void);
void spi_get_values(uint16_t *temperature, uint32_t *pressure);

#endif
