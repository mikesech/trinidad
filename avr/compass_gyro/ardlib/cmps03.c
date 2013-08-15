//cmps03.c  - functions for controlling CMPS03 compass sensor from AVR Atmega64 (Cerebot) using I2C bus

#include <inttypes.h>

// this is included from AVRlib (link with i2c.c)
#include "i2c.h"

//compass has fixed I2C address C0
#define COMPASS_ADDRESS 0xC0

// Returns direction as 1 byte (0-255)
uint8_t cmps03_direction_byte(void)
{
  uint8_t buf[1];
  uint8_t bearing;
  buf[0] = 0x1;   // compass bearing as a byte, register 1
  i2cMasterSendNI(COMPASS_ADDRESS, 1, buf);
  i2cMasterReceiveNI(COMPASS_ADDRESS, 1, &bearing);
  return bearing;
}

// Returns direction in tenths of degree (0-3599)
uint16_t cmps03_direction(void)
{
  uint8_t buf[1];
  uint8_t bearing1, bearing2;
  buf[0] = 0x2;   // compass bearing as a word, register 2 - high byte
  i2cMasterSendNI(COMPASS_ADDRESS, 1, buf);
  i2cMasterReceiveNI(COMPASS_ADDRESS, 1, &bearing1);
  buf[0] = 0x3;   // compass bearing as a word, register 3 - low byte
  i2cMasterSendNI(COMPASS_ADDRESS, 1, buf);
  i2cMasterReceiveNI(COMPASS_ADDRESS, 1, &bearing2);

  return ((uint16_t)bearing1 << 8) + bearing2;
}

// Compass calibration: must be called 4-times for all four directions North, East, South, West precisely!
// You have to use real compass to compare to the values. Calibration is stored in the compass and does
// not need to be performed more than once at specific geographic location
void cmps03_calibrate(void)
{
  uint8_t buf[2];
  buf[0] = 15;   // compass calibration, register 15
  buf[1] = 255;
  i2cMasterSendNI(COMPASS_ADDRESS, 2, buf);
}

