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

#ifndef PURT_STATEDEVICE_BUFFER_H
#define PURT_STATEDEVICE_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "modes.h"

#ifndef PURT_MODE
#define PURT_MODE PURT_SD_BUFFER
#endif

#if PURT_IS_SD_BUFFER()
/* Start of actual header file */
#include "src/statedevice_common.h"

/* Initialization stuff */
void purt_sd_buffer_init_enc(unsigned int enc_baud);
#define purt_sd_buffer_init() \
	purt_sd_buffer_init_enc((F_CPU +  PURT_STATEDEVICE_BAUD * 8L) / \
	( PURT_STATEDEVICE_BAUD * 16L) - 1)
	
/* Other stuff */
void purt_sd_buffer_process_message(void);

/* End of actual header file */
#else /* !PURT_IS_SD_BUFFER() */
#warning "statedevice_buffer.h" has been included even though in other mode.
#endif

#ifdef __cplusplus
}
#endif

#endif
