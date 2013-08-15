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

#define PURT_STATEDEVICE_COMMON_C
#include "statedevice_common.h"
#include "target/serial.h"

void purt_ask_substate(const char* key, unsigned char keyLength) {
	const unsigned char size = 3 + keyLength;
	PURT_UART_BLOCK_SET_BYTE(size);
	PURT_UART_BLOCK_SET_BYTE(0x01);
	PURT_UART_BLOCK_SET_BYTE(keyLength);
	unsigned char i;
	for(i = 0; i < keyLength; i++)
		PURT_UART_BLOCK_SET_BYTE(key[i]);
	PURT_UART_BLOCK_SET_BYTE(~size);
}
void purt_set_substate(const char* key, unsigned char keyLength, const char* value, unsigned char valueLength) {
	const unsigned char size = 3 + keyLength + valueLength;
	PURT_UART_BLOCK_SET_BYTE(size);
	PURT_UART_BLOCK_SET_BYTE(0x00);
	PURT_UART_BLOCK_SET_BYTE(keyLength);
	unsigned char i;
	for(i = 0; i < keyLength; i++)
		PURT_UART_BLOCK_SET_BYTE(key[i]);
	for(i = 0; i < valueLength; i++)
		PURT_UART_BLOCK_SET_BYTE(value[i]);
	PURT_UART_BLOCK_SET_BYTE(~size);
}
void purt_register_substate(const char* key, unsigned char keyLength) {
	const unsigned char size = 3 + keyLength;
	PURT_UART_BLOCK_SET_BYTE(size);
	PURT_UART_BLOCK_SET_BYTE(0x02);
	PURT_UART_BLOCK_SET_BYTE(keyLength);
	unsigned char i;
	for(i = 0; i < keyLength; i++)
		PURT_UART_BLOCK_SET_BYTE(key[i]);
	PURT_UART_BLOCK_SET_BYTE(~size);
}
