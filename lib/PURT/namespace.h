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

#ifndef PURT_NAMESPACE_H
#define PURT_NAMESPACE_H

/* raw.h */
#define uart_init_enc 		purt_uart_init_enc
#define uart_init 			purt_uart_init
#define uart_init_stdio		purt_uart_init_stdio
#define uart_recv 			purt_uart_recv
#define uart_send 			purt_uart_send
#define uart_input_waiting	purt_uart_input_waiting
#define uart_print 			purt_uart_print
#define uart_get_token		purt_uart_get_token
#define uart_get_delim		purt_uart_get_delim
#define uart_print_int		purt_uart_print_int
#define uart_print_uint		purt_uart_print_uint
#define uart_get_int		purt_uart_get_int
#define uart_get_uint		purt_uart_get_uint

#endif
