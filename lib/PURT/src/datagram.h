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

#ifndef PURT_DATAGRAM_H
#define PURT_DATAGRAM_H

#include "substate.h"

/** POD holding components of a datagram. */
typedef struct {
	unsigned char type;
	unsigned char message[255];
	unsigned char length;
} Purt_Datagram;

/** Gets a datagram over serial link.
  * @return Pointer to internally-buffered datagram. Will change after next serial library call. NULL if error.
  */
Purt_Datagram* purt_get_datagram(void);

/** Sends a datagram over serial link.
  * @param type type of datagram
  * @param msg message of datagram
  * @param length length of message
  */
void purt_send_datagram(unsigned char type, const unsigned char* msg, unsigned char length);

/** Extracts a Substate from a given Purt_Datagram.
  * @param d Datagram from which to extract Purt_Substate
  * @return Pointer to internal-buffer datagram. Will change after next serial library call or if given Purt_Datagram modified.
  *	NULL if given null pointer.
  */
Purt_Substate* purt_extract_substate(const Purt_Datagram* d);

#endif
