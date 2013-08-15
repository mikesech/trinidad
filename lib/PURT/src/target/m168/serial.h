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

/* Defines device primitives for serial port use. Abstracts concepts so that *
 * the rest of PURT can be device-agnostic. For atmega168.					 */
 
#ifndef PURT_SERIAL_TARGET_H
#error Do not include m168/serial.h directly. Instead, include target/serial.h.
#elif !defined(PURT_SERIAL_M168_H)
#define PURT_SERIAL_M168_H

#include <avr/io.h>
#include <avr/interrupt.h>

static inline void PURT_UART_ACTIVATE(unsigned int encBaud) {
	UBRR0 = encBaud;
	UCSR0B = ( (1<<RXEN0) | (1<<TXEN0) );
	UCSR0C = 0b00000110;
}

static inline void PURT_UART_WAIT_INPUT(void) {
	while ( !(UCSR0A & (1<<RXC0)) );
}
	
static inline unsigned char PURT_UART_GET_BYTE(void) {
	return UDR0;
}

static inline void PURT_UART_WAIT_READY_SEND(void) {
	while ( !(UCSR0A & (1<<UDRE0)) );
}

static inline void PURT_UART_SET_BYTE(unsigned char c) {
	UDR0 = c;
}

static inline unsigned char PURT_UART_HAS_INPUT(void) {
	return UCSR0A & (1<<RXC0);
}

#define PURT_UART_RX_VECTOR() ISR(USART_RX_vect)

static inline void PURT_UART_DISABLE_RX_ISR(void) {
	UCSR0B &= 0b01111111;
}

static inline void PURT_UART_ENABLE_RX_ISR(void) {
	UCSR0B |= 0b10000000;
}

static inline void PURT_ENABLE_ISRS(void) {
	sei();
}

static inline void PURT_DISABLE_ISRS(void) {
	cli();
}

#endif
