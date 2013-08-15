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

#ifndef PRIVSERIAL_H
#define PRIVSERIAL_H

/** @file
  * Declares private serial-related functions that should only be used by MiniURT
  * internal functions.
  */

#ifndef F_CPU
#error F_CPU must be defined
#endif

#include <stdint.h>

/** Initalizes the UART. Must be called before any other serial-related functions.
  * @param encBaud baud rate in bps
  */
void initUART(uint16_t baud);

/** Recieves one byte; waits for one to arrive if needed.
  * @return actual byte recieved
  */
unsigned char receiveByte(void);
/** Sends one byte; does not return until UART is free to transmit.
  * @param data byte to send
  */
void transmitByte(unsigned char data);

#endif
