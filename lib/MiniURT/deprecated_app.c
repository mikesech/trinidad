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

#include "miniurt.h"

#warning "The use of deprecated_app is deprecated. Please use the suggest interrupt-based system."

void onPoll(void);

int main(void) {
	initialize();
	while(1) {
		if(polled())
			onPoll();
	}
}
