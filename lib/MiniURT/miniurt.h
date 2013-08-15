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

#ifndef MINIURT_H
#define MINIURT_H

#include <stdbool.h>

#ifndef F_CPU
#error F_CPU must be defined
#endif

/** @mainpage
  * See miniurt.h documentation for general information.
  */

/** @file
  * miniurt.h defines all necessary functions to interact with a URT-based server.
  * The new way to communicate with the server is with an interrupt-based system.
  * In general, client code should implement a main function and constantly do work
  * (poll sensors, make calculations, etc.) and update global variables with current data.
  * When the server interacts with this device, execution will be interrupted and either
  * onInit() or onIncoming() will be called to handle post-handshake activity or incoming substate information, respectively.
  * As in URT_avr, client code also needs to define the <tt>const unsigned char</tt> APPLICATION_TYPE
  * and UID.
  *
  * Note that there is no longer an onPoll() function. Instead, client code should periodically
  * call polled() to determine whether a poll has been received since the last time it was called.
  * Since a device is under no obligation to service every poll, it is expected that some polls
  * will not in fact be answered. Instead, client code should check for a request after every time
  * sensory (or other) data has been updated and service it accordingly.
  *  
  *  Since this system is interrupt based, it is important to create interrupt safe code.
  *	In general, when accessing any variable that is used in onInit() or onIncoming()
  *	in either the main function or a function called by main, place said code in an atomic block
  *	as follows:
  * @code
  *	ATOMIC_BLOCK(ATOMIC_FORCEON) {
  *		sharedVariable = 1;
  *	}
  *  @endcode
  *  For optimal performance, try to limit the code in the block to only assignments. Do all calculations
  *	just before. Furthermore, remember to include <util/atomic.h>.
  *
  * @note The old URT_avr non-interrupt method of supplying only an onInit(), onPoll(), and onIncoming()
  *	and not a main function is supported by MiniURT, although unadvised, by compiling and linking with
  *	deprecated_app.c.
  *
  * @see \ref example.c "MiniURT example usage: example.c"
  */

/** POD holding pointers to components of substate. Generally associated with a particular Datagram. */
typedef struct {
	const unsigned char* key;
	unsigned char keyLength;
	const unsigned char* value;
	unsigned char valueLength;
} Substate;

/** Requests a Substate over serial link.
  * @param key key of substate
  * @param keyLength length of key
  */
void askSubstate(const void* key, unsigned char keyLength);
/** Sets a Substate over serial link.
  * @param key key
  * @param keyLength key length
  * @param value value
  * @param valueLength value length
  */
void setSubstate(const void* key, unsigned char keyLength, const void* value, unsigned char valueLength);

/** Checks to see if a poll request has been received since the last time this function was called */
bool polled(void);

/** Macro to easily use string literals with above functions that
  * take both string pointer and string length. Only use inline with string
  * literals: _("word").
 */
#define _(x) x,(sizeof(x)-1)

/** Initializes the URT system. Should be called after initialization of all other components in main(). */
void initialize(void);
void initialize_baud(unsigned int baud);
#endif
