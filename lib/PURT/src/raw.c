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

#include "../raw.h"
#include "target/serial.h"
#include <stdlib.h>
#include <ctype.h>

/* Initialization stuff */
inline void purt_uart_init_enc(unsigned int enc_baud) {
	PURT_UART_ACTIVATE(enc_baud);
}
	
/* Byte I/O */
inline unsigned char purt_uart_recv() {
	return PURT_UART_BLOCK_GET_BYTE();
}
inline void purt_uart_send(unsigned char byte) {
	PURT_UART_BLOCK_SET_BYTE(byte);
}
inline bool purt_uart_input_waiting() {
	return PURT_UART_HAS_INPUT();
}

/* String I/O */
void purt_uart_print(const char* string) {
	while(*string != '\0') purt_uart_send(*(string++));
}
char* purt_uart_get_token(char* buffer, unsigned char maxSize) {
	if(!maxSize) return buffer;
	
	do {
		*buffer = purt_uart_recv();
	} while(!isspace(*(buffer++)) && --maxSize);
	*buffer = '\0';
	return buffer;	
}
char* purt_uart_get_delim(char* buffer, unsigned char maxSize, char delim) {
	if(!maxSize) return buffer;
	
	do {
		*buffer = purt_uart_recv();
		if(*buffer == delim) break;
		++buffer;
	} while(--maxSize);

	*buffer = '\0';
	return buffer;
}

/* Integer I/O */
void purt_uart_print_int(int i) {
	char out[7];
	purt_uart_print(itoa(i, out, 10));
}
void purt_uart_print_uint(unsigned int i) {
	char out[6];
	purt_uart_print(utoa(i, out, 10));
}
int purt_uart_get_int(void) {
	char a[7];
	purt_uart_get_token(a, 6);
	return (int)strtol(a, (char **)NULL, 10);
}
unsigned int purt_uart_get_uint(void) {
	char a[7];
	purt_uart_get_token(a, 6);
	return (int)strtoul(a, (char **)NULL, 10);
}


