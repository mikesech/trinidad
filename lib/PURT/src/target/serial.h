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

#ifndef PURT_SERIAL_TARGET_H
#define PURT_SERIAL_TARGET_H

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__)
#include "m168/serial.h"
#else
#error This library does not presently support the serial capabilities of this platform.
#endif

//Common compositions
static inline unsigned char PURT_UART_BLOCK_GET_BYTE(void) {
	PURT_UART_WAIT_INPUT();
	return PURT_UART_GET_BYTE();
}

static inline void PURT_UART_BLOCK_SET_BYTE(unsigned char c) {
	PURT_UART_WAIT_READY_SEND();
	PURT_UART_SET_BYTE(c);
}

#endif

