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

#include "../statedevice_direct.h"
#include "target/serial.h"
#include "datagram.h"

void purt_sd_direct_init_enc(unsigned int enc_baud) {
	PURT_UART_ACTIVATE(enc_baud);
}
void purt_sd_direct_process_message(void) {
	Purt_Datagram* d = purt_get_datagram();
	if(d == 0) { /* Invalid datagram; ignore */
		return;
	} else if(d->type == 0) { /* Server is polling. Set all possible substates. */
		onPoll();
	} else if(d->type == 0x01) { /* Server is responding (or unsolicitedly sending) a substate. Handle extracted datagram. */
		onSubstate(purt_extract_substate(d));
	} else if(d->type == 0xFF) { /* Handshake is occuring. */
		const unsigned char handshake[] = {APPLICATION_TYPE, UID};
		purt_send_datagram(0xFF, handshake, 2);
		onHandshake();
	}
}
