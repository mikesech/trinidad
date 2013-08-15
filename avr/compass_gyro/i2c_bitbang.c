#include "i2c_bitbang.h"
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

inline bool READSDA(void) {
	DDRC &= (~(1 << 5));
	return PINC & (1 << 5);
}
inline void CLRSDA(void) {
	DDRC |= (1 << 5);
	PORTC &= (~(1 << 5));
}
inline bool READSCL(void) {
	DDRC &= (~(1 << 4));
	return PINC & (1 << 4);
}
inline void CLRSCL(void) {
	DDRC |= (1 << 4);
	PORTC &= (~(1 << 4));
}

#define I2CDELAY _delay_us
#define I2CSPEED 10 //interval between bits in microseconds

bool start = 0;
bool read_bit(void)
{
	bool bit;
 
	//Let slave drive data signal.
	READSDA();
	I2CDELAY(I2CSPEED/2);
	/* Clock stretching */
	while (READSCL() == 0);
	/* SCL is high, now data is valid */
	bit = READSDA();
	I2CDELAY(I2CSPEED/2);
	CLRSCL();
	return bit;
}
 
bool write_bit(bool bit)
{
	if(bit) 
		READSDA();
	else 
		CLRSDA();
	I2CDELAY(I2CSPEED/2);
	/* Clock stretching */
	while (READSCL() == 0);
	/* SCL is high, now data is valid */
	/* check that nobody is driving SDA */
	if (bit && READSDA() == 0)
		return false; //arbitration lost
	I2CDELAY(I2CSPEED/2);
	CLRSCL();
	return true;
}
 
bool start_cond(void)
{
	if (start) {
		/* set SDA to 1 */
		READSDA();
		I2CDELAY(I2CSPEED/2);
		/* Clock stretching */
		while (READSCL() == 0);
	}
	if (READSDA() == 0)
		return false; //arbitration lost
	/* SCL is high, set SDA from 1 to 0 */
	CLRSDA();
	I2CDELAY(I2CSPEED/2);
	CLRSCL();
	start = 1;
	return true;
}
 
bool stop_cond(void)
{
	/* set SDA to 0 */
	CLRSDA();
	I2CDELAY(I2CSPEED/2);
	/* Clock stretching */
	while (READSCL() == 0);
	/* SCL is high, set SDA from 0 to 1 */
	if (READSDA() == 0)
		return false; //arbitration lost
	I2CDELAY(I2CSPEED/2);
	start = 0;
	return true;
}
 
bool tx(bool send_start, bool send_stop, unsigned char byte)
{
	unsigned bit;
	unsigned nack;
 
	if (send_start)
		start_cond();
	for (bit = 0; bit < 8; bit++) {
		write_bit(byte & 0x80);
		byte <<= 1;
	}
	nack = read_bit();
	if (send_stop)
		stop_cond();
 
	return nack;
}
 
unsigned char rx(bool nak, bool send_stop)
{
	unsigned char byte = 0;
	unsigned bit;
 
	for (bit = 0; bit < 8; bit++) {
		byte |= read_bit();
		byte <<= 1;
	}
	write_bit(nak);
	if (send_stop)
		stop_cond();
	return byte;
}

