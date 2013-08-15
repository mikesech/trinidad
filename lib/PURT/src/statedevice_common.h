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

#ifndef PURT_STATEDEVICE_COMMON_H
#define PURT_STATEDEVICE_COMMON_H

#if !defined(PURT_STATEDEVICE_DIRECT_H) && !defined(PURT_STATEDEVICE_BUFFER_H)\
	&& !defined(PURT_STATEDEVICE_COMMON_C)
#error Do not include statedevice_common.h directly. Instead, include \
	statedevice_direct.h or statedevice_buffer.h depending on which mode you \
	desire.
#else

/* Start of actual header file */
#include "substate.h"
/* Constants */
#define PURT_STATEDEVICE_BAUD 2400

/** Requests a Substate over serial link.
  * @param key key of substate
  * @param keyLength length of key
  */
void purt_ask_substate(const char* key, unsigned char keyLength);

/** Sets a Substate over serial link.
  * @param key key
  * @param keyLength key length
  * @param value value
  * @param valueLength value length
  */
void purt_set_substate(const char* key, unsigned char keyLength, const char* value, unsigned char valueLength);

/** Registers a Substate for a push relationship over serial link.
  * The server will automatically send a registered substate when it changes
  * (unless the substate has been marked as send-on-touch on the server) as if
  * this device asked for it.
  *
  * @note It is unadvisable to register a substate multiple times for a
  *	particular session, since the server will send a duplicate update for
  *	each registration. Generally, the device should only register substates
  *	in onInit(), which is called upon the initalization of each session.
  *
  * @param key key of substate
  * @param keyLength length of key
  */
void purt_register_substate(const char* key, unsigned char keyLength);

/** Macro to easily use string literals with above functions that
  * take both string pointer and string length. Only use inline with string
  * literals: _("word").
 */
#define _(x) x,(sizeof(x)-1)

/* Client-code supplied */
void onHandshake(void);
void onSubstate(Purt_Substate* s);
void onPoll(void);
extern const unsigned char APPLICATION_TYPE;
extern const unsigned char UID;

#endif /* check for inclusion by statedevice_direct/statedevice_buffer */
#endif /* include guard */
