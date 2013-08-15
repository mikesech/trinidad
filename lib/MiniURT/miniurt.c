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
#include <avr/interrupt.h>
#include <stdbool.h>
#include "miniurt.h"
#include "private_serial.h"

void onInit(void);
void onIncoming(Substate* s);
extern const unsigned char APPLICATION_TYPE;
extern const unsigned char UID;

void initialize_baud(unsigned int baud) {
	initUART(baud);
	UCSR0B |= 0b10000000; //activate receive interrupt
	sei();
}

void initialize(void) {
	initialize_baud(2400);
}

static bool receivedPoll = false;
bool polled(void) {
	bool temp = receivedPoll;
	receivedPoll = false;
	return temp;
}

ISR(USART_RX_vect) {
	unsigned char input = UDR0;

	UCSR0B &= 0b01111111; //disable receive interrupt so we don't reenter
	sei(); //reenable interrupts
	
	static unsigned char buffer[255];
	static unsigned char* ptr = buffer;
	static unsigned char size = 0, bytesRemaining = 0;
	static bool initialized = false;
	static Substate substate;
	
	
	if(bytesRemaining > 0) { //continuing datagram
		*ptr = input;
		if(--bytesRemaining == 0) { //datagram completely received
			if(size == (unsigned char)(~*ptr)) { //intact
				switch(buffer[0]) {
					case 0x00:
						receivedPoll = true;
						break;
					case 0x01:
						substate.keyLength = buffer[1];
						substate.valueLength = size - substate.keyLength - 1;
						substate.key = &buffer[2];
						substate.value = substate.key + substate.keyLength;
						onIncoming(&substate);
						break;
					case 0xFF:
						transmitByte(4);
						transmitByte(0xFF);
						transmitByte(APPLICATION_TYPE);
						transmitByte(UID);
						transmitByte(~4);
					
						if(!initialized) {
							onInit();
							initialized = true;
						}
						break;
				}
			}
			ptr = buffer;
		} else
			ptr++;
	} else if(input > 1) //new datagram (and not meaningless interstitial data)
		bytesRemaining = size = input;
	
	//disable interrupts so that a serial interrupt won't happen until we return (and interrupts are automatically enabled) after our stack memory is released
	cli();
	UCSR0B |= 0b10000000; //re-enable interrupts
}
