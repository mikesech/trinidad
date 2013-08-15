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

#ifndef PURT_RAW_H
#define PURT_RAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "modes.h"
#include <stdbool.h>

#ifndef PURT_MODE
#define PURT_MODE PURT_RAW
#endif

#if PURT_IS_RAW()
/* Start of actual header file */

/* Initialization stuff */
void purt_uart_init_enc(unsigned int enc_baud);
#define purt_uart_init(baudrate) \
	purt_uart_init_enc((F_CPU + baudrate * 8L) / (baudrate * 16L) - 1)
void purt_uart_init_stdio(void);
	
/* Byte I/O */
unsigned char purt_uart_recv(void);
void purt_uart_send(unsigned char byte);
bool purt_uart_input_waiting(void);

/* String I/O */
void purt_uart_print(const char* string);
char* purt_uart_get_token(char* buffer, unsigned char maxSize);
char* purt_uart_get_delim(char* buffer, unsigned char maxSize, char delim);

/* Integer I/O */
void purt_uart_print_int(int i);
void purt_uart_print_uint(unsigned int i);
int purt_uart_get_int(void);
unsigned int purt_uart_get_uint(void);

/* End of actual header file */
#else /* PURT_IS_SD() */
#error "raw.h" has been included even though in SD mode.
#endif


#ifdef __cplusplus
}
#endif

#endif

