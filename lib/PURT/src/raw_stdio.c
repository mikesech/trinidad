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
#include <stdio.h>

static int uart_putchar(char c, FILE *stream)
{
	(void)stream; //suppress unused warning
	PURT_UART_BLOCK_SET_BYTE(c);
	return 0;
}

static int uart_getchar(FILE *stream)
{
	(void)stream; //suppress unused warning
	return PURT_UART_BLOCK_GET_BYTE();
}

static FILE in = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
static FILE out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void purt_uart_init_stdio() {
	stdin = &in;
	stdout = &out;
	stderr = &out;
}

