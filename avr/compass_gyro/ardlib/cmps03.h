//cmps03.h  - functions for controlling CMPS03 compass sensor from AVR Atmega64 (Cerebot) using I2C bus

#ifndef __cmps03_h__
#define __cmps03_h__

// Returns direction as 1 byte (0-255)
uint8_t cmps03_direction_byte(void);

// Returns direction in tenths of degree (0-3599)
uint16_t cmps03_direction(void);

// Compass calibration: must be called 4-times for all four directions North, East, South, West precisely!
// You have to use real compass to compare to the values. Calibration is stored in the compass and does
// not need to be performed more than once at specific geographic location
void cmps03_calibrate(void);


#endif
