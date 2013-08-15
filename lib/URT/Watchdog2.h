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
 * Watchdog.h
 *
 *  Created on: Mar 17, 2010
 *      Author: Michael Sechooler
 */

#ifndef WATCHDOG2_H_
#define WATCHDOG2_H_

#include <string>
#include "Timer.h"
#include "State.h"

namespace urt {

/**
 * Watchdog2 is a simple wrapper class for Timer that enables watching given substates and generating an event if none
 * of them have been touched within the last given time period. To use this class, extend it and implement onTrip() 
 * with the actions you  want done on a timeout. Note that a timeout only occurs once when the timer first
 * exceeds \c timeout; if there is no later check-in, another timeout event will not be generated. Furthermore,
 * note that substate changes/touches are ignored while in onTrip() to enable changing the watched substates
 * without reseting the watchdog.
 *
 * @attention Watchdog2 sets the associated key(s) to trigger its/their signal(s) even when only touched. Should you change this
 * 	setting after creating the Watchdog2, touching a substate by setting it to its already-set value will not qualify
 *	as checking in and will not reset the watchdog timer.
 */
class Watchdog2 : public Timer {
public:
	/**
	 * Creates and activates a watchdog for a particular key, associating it with the given EventLoop.
	 * @param timeout The timeout in milliseconds.
	 */
	Watchdog2(unsigned int timeout) : Timer(timeout), m_ignoreSubstateReset(false) {}

	/**
	 * Associates watchdog with an additional key.
	 * @param key key to associate with
	 */
	void associateKey(const std::string& key) {
		State::setSignalOnTouch(key, true);
		State::registerSlot(key, &Watchdog2::reset, *this);
	}
	/**
	  * Assoicates watchdog with an additional key and allows chaining.
	  * @param key Key to assoicate with.
	  * @return Reference to this.
	  */
	Watchdog2& operator<<(const std::string& key) { associateKey(key); return *this; }

	/**
	 * Pure virtual function called when a watchdog timeout occurs. Extend class
	 * and implement to define what happens when a watchdog times out.
	 */
	virtual void onTrip() = 0;

	/**
	 * Resets watchdog timer. Takes two string parameters (but does nothing with them)
	 * to enable use as Substate slot. Note that resets through this method are ignored
	 * while in onTrip() to enable changing the watched substates without triggering the watchdog.
	 */
	void reset(const std::string& k, const std::string& v) { if(!m_ignoreSubstateReset) Timer::reset(); }
	
	//Unhide parent's reset(), which is hidden by the above overload.
	using Timer::reset;
	
private:
	bool onTimeout() {
		stop();
		m_ignoreSubstateReset = true;
		onTrip();
		m_ignoreSubstateReset = false;
		return true;
	};
	bool m_ignoreSubstateReset;
};

}

#endif /* WATCHDOG2_H_ */
