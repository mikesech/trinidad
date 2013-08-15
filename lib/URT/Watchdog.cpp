/* Copyright 2009-2011 Michael Sechooler
 *
 * This file is part of URT.
 * 
 * URT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * URT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with URT.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Watchdog.cpp
 *
 *  Created on: Mar 17, 2010
 *      Author: Michael Sechooler
 */

#include "Watchdog.h"

#include "State.h"
#include "EventLoop.h"

namespace urt {

Watchdog::Watchdog(EventLoop& l, unsigned int timeout, const std::string& key) : timeout(timeout), hasTimedout(false), resetsEnabled(true) {
	reset();
	l.registerIntervalSlot(&Watchdog::check, *this);
	associateKey(key);
}

void Watchdog::associateKey(const std::string& key) {
	State::setSignalOnTouch(key, true);
	State::registerSlot(key, &Watchdog::reset, *this);
}

void Watchdog::reset() {
	if(resetsEnabled)
		clock_gettime(CLOCK_MONOTONIC,&lastCheckIn);
}

void Watchdog::check() {
		timespec now;
		clock_gettime(CLOCK_MONOTONIC,&now);

		time_t diffSeconds = now.tv_sec - lastCheckIn.tv_sec;
		long diffNano = now.tv_nsec - lastCheckIn.tv_nsec;
		unsigned int diffMilli = diffSeconds * 1000 + diffNano / 1000000;
		if(diffMilli >= timeout) {
			if(!hasTimedout) //prevent it from firing more than once per timeout
				onTimeout();
			hasTimedout = true;
		} else
			hasTimedout = false;
}

}

