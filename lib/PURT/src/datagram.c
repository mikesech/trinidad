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

#include "datagram.h"
#include "target/serial.h"

Purt_Datagram* purt_get_datagram() {
	static Purt_Datagram buffer;
	
	unsigned char size;
	do {
		size = PURT_UART_BLOCK_GET_BYTE();
	} while(size < 2); /* Size must be greater than 1, otherwise it is meaningless interstitial data */
	unsigned char size2 = ~size;
	buffer.length = size - 2;
	buffer.type = PURT_UART_BLOCK_GET_BYTE();
	unsigned char count;
	for(count = 0; count < buffer.length; count++)
		buffer.message[count] = PURT_UART_BLOCK_GET_BYTE();
	if(PURT_UART_BLOCK_GET_BYTE() == size2)
		return &buffer;
	else
		return 0;
}

void purt_send_datagram(unsigned char type, const unsigned char* msg, unsigned char length) {
	PURT_UART_BLOCK_SET_BYTE(length + 2);
	PURT_UART_BLOCK_SET_BYTE(type);
	unsigned char i;
	for(i = 0; i < length; i++)
		PURT_UART_BLOCK_SET_BYTE(msg[i]);
	PURT_UART_BLOCK_SET_BYTE(~(length + 2));
}

Purt_Substate* purt_extract_substate(const Purt_Datagram* d) {
	static Purt_Substate buffer;

	if(d == 0)
		return 0;
	
	buffer.keyLength = d->message[0];
	buffer.valueLength = d->length - buffer.keyLength - 1;
	buffer.key = &d->message[1];
	buffer.value = buffer.key + buffer.keyLength;
	
	return &buffer;
}
