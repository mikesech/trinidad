/* Copyright 2009-2011 Michael Sechooler
 *
 * This file is part of PURT.
 * 
 * PURT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * PURT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with PURT.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../statedevice_buffer.h"
#include "target/serial.h"
#include "substate.h"
#include <stdbool.h>

/* Message Queue interface and implementation */
/* The message queue stores a unique list of the three message
 * types (NONE is special). */
typedef enum {
	NONE,
	POLL,
	SUBSTATE,
	HANDSHAKE
} MessageType;
static volatile MessageType queue[] = { NONE, NONE, NONE };
static void push(MessageType t) {
	if	   (queue[0] == NONE) queue[0] = t;
	else if(queue[0] == t   ) return;
	else if(queue[1] == NONE) queue[1] = t;
	else if(queue[1] == t   ) return;
	else                      queue[2] = t;
}
static void atomic_pop(void) {
	PURT_UART_DISABLE_RX_ISR();
	queue[0] = queue[1];
	queue[1] = queue[2];
	queue[2] = NONE;
	PURT_UART_ENABLE_RX_ISR();
}
static bool exists(MessageType t) {
	return queue[0] == t || queue[1] == t || queue[2] == t;
}

/* File-specific static variables */
static Purt_Substate substate;

/* Initialization functions */
void purt_sd_buffer_init_enc(unsigned int enc_baud) {
	PURT_UART_ACTIVATE(enc_baud);
	PURT_UART_ENABLE_RX_ISR();
	PURT_ENABLE_ISRS();
}

/* Message processing functions */
void purt_sd_buffer_process_message(void) {
	switch(queue[0]) {
	case POLL:
		atomic_pop();		
		onPoll();
		break;
	case SUBSTATE:
		onSubstate(&substate);
		atomic_pop();
		break;
	case HANDSHAKE:
		atomic_pop();
		
		PURT_UART_BLOCK_SET_BYTE(4);
		PURT_UART_BLOCK_SET_BYTE(0xFF);
		PURT_UART_BLOCK_SET_BYTE(APPLICATION_TYPE);
		PURT_UART_BLOCK_SET_BYTE(UID);
		PURT_UART_BLOCK_SET_BYTE(~4);

		onHandshake();
		break;
	case NONE:
		break;
	}
}

PURT_UART_RX_VECTOR() {
	const unsigned char input = PURT_UART_GET_BYTE();

	PURT_UART_DISABLE_RX_ISR(); //disable receive interrupt so we don't reenter
	PURT_ENABLE_ISRS(); //reenable interrupts
	
	static unsigned char buffer[253];
	static unsigned char* ptr = buffer;
	static unsigned char size = 0, bytesRemaining = 0;
	static unsigned char type;
	static bool drop = false; //if true, ignore current datagram
	
	if(bytesRemaining > 0) { //continuing the datagram
		if(bytesRemaining-- == size) {
			//this is the second byte (type)
			type = input;
			
			if(type == 0x01) {
				//if the buffer is currently in use by another substate, drop
				//the incoming one
				if(exists(SUBSTATE)) drop = true;
			} else {
				//We don't want to accidentally write to the buffer, so force
				//the bytes remaining to 1 (no message, just terminator).
				//Currently, this is compatible with the protocol and protects
				//against malformed packets from overwritting the buffer by
				//skipping the substate check above.
				bytesRemaining = 1;
			}
		} else if(!drop) {
			if(bytesRemaining == 0) { //datagram completely received
				if(size == (unsigned char)(~input)) { //intact
					switch(type) {
						case 0x00:
							push(POLL);
							break;
						case 0x01:
							substate.keyLength = buffer[1];
							substate.valueLength = size - substate.keyLength - 1;
							substate.key = &buffer[2];
							substate.value = substate.key + substate.keyLength;
						
							push(SUBSTATE);
							break;
						case 0xFF:
							push(HANDSHAKE);
							break;
					}
				}
				drop = false;
				ptr = buffer;
			} else
				*ptr++ = input;
		}
	} else if(input > 1) //new datagram (and not meaningless interstitial data)
		bytesRemaining = size = input;
	
	//disable interrupts so that a serial interrupt won't happen until we return
	//(and interrupts are automatically enabled) after our stack memory is released
	PURT_DISABLE_ISRS();
	PURT_UART_ENABLE_RX_ISR(); //re-enable interrupts
}

