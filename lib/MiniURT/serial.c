/* Copyright 2009-2011 Michael Sechooler
 *
 * This file is part of MiniURT.
 * 
 * MiniURT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * MiniURT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MiniURT.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include "miniurt.h"
#include "private_serial.h"

/* UART functions (read, write, check)*/
// Initialize the serial port (UART)
void initUART( uint16_t baud )
{
	UBRR0 = ((F_CPU +  baud * 8L) / ( baud * 16L) - 1);
	UCSR0B = ( (1<<RXEN0) | (1<<TXEN0) );  //enable UART receiver and transmitter
	UCSR0C = 0b00000110;
}

/* Read and write functions */
unsigned char receiveByte( void )
{
	while ( !(UCSR0A & (1<<RXC0)) ); // wait for data in buffer
	return UDR0; //return data from buffer
}
void transmitByte( unsigned char data )
{
	while ( !(UCSR0A & (1<<UDRE0)) ); //wait for UART ready to send
	UDR0 = data; //send data
}

void askSubstate(const void* key, unsigned char keyLength) {
	unsigned char size = 3 + keyLength;
	transmitByte(size);
	transmitByte(0x01);
	transmitByte(keyLength);
	unsigned char i;
	for(i = 0; i < keyLength; i++)
		transmitByte(((const char*)key)[i]);
	transmitByte(~size);
}
void setSubstate(const void* key, unsigned char keyLength, const void* value, unsigned char valueLength) {
	unsigned char size = 3 + keyLength + valueLength;
	transmitByte(size);
	transmitByte(0x00);
	transmitByte(keyLength);
	unsigned char i;
	for(i = 0; i < keyLength; i++)
		transmitByte(((const char*)key)[i]);
	for(i = 0; i < valueLength; i++)
		transmitByte(((const char*)value)[i]);
	transmitByte(~size);
}
