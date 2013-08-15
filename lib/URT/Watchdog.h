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

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <string>
#include <boost/utility.hpp>
#include <boost/signals/trackable.hpp>
#include <ctime>

//class EventLoop;
#include "EventLoop.h"

//attributes are only allowed by gcc
#ifdef __GNUC__
#define DEPRECATED __attribute__ ((deprecated))
#else
#define DEPRECATED
#endif

namespace urt {

/**
 * The Watchdog abstract class keeps track of given substates in the global State class and
 * generates an event if none of them have been touched within the last given time period.
 *
 * @deprecated Watchdog2 provides a more fine-grained watchdog system. Use it instead.
 *
 * This class allows actions to be taken if, for example, communication has been cut off. Each Watchdog
 * object is associated with substate(s). Client code can check-in and reset the Watchdog timer by calling reset()
 * or touching the associated substate(s).
 *
 * @attention Watchdog sets the associated key(s) to trigger its/their signal(s) even when only touched. Should you change this
 * 	setting after creating the Watchdog, touching a substate by setting it to its already-set value will not qualify
 *  as checking in and will not reset the watchdog timer.
 *
 * The Watchdog timer is checked every time the associated EventLoop interval handler is triggered. Therefore,
 * the Watchdog only works when the EventLoop is running and has a granularity limited to EventLoop::timeout.
 *
 * To use this class, extend it and implement onTimeout() with the actions you want done on a timeout. Note that
 * a timeout only occurs once when the timer first exceeds \c timeout; if there is no later check-in, another timeout
 * event will not be generated.
 *
 * @note The Watchdog is not a FDEvtSource; instead, it attaches to the EventLoop interval handler. As such, it is not managed
 * by the EventLoop and will not be deleted by it. It may therefore be created on the stack. When the Watchdog dies, the
 * connection with the EventLoop will be automatically severed.
 */
class Watchdog : public boost::signals::trackable, boost::noncopyable {
public:
	/**
	 * Creates and activates a watchdog for a particular key, associating it with the given EventLoop.
	 * @param l The EventLoop whose interval handler to attach.
	 * @param timeout The timeout in microseconds. If, when checking the status of the Watchdog, no
	 * 	reset has been recorded in the last timeout duration, onTimeout() will be called. Note that
	 * 	an actual duration is guaranteed to be no less then \c timeout and generally not greater than
	 * 	\c timeout plus the EventLoop interval timeout.
	 * @param key The associated key that triggers a reset of the watchdog timer.
	 */
	Watchdog(EventLoop& l, unsigned int timeout, const std::string& key = "URTWatchdog") DEPRECATED;
	/**
	 * Creates and activates a watchdog for a set of keys. The watchdog timer resets every time
	 * one of the keys is touched.
	 * @param l The EventLoop whose interval handler to attach.
	 * @param timeout The timeout in microseconds.
	 * @tparam InputIterator type of iterator for container of either C-strings or std::strings
	 * @param begin InputIterator at beginning of keys container
	 * @param end InputIterator at end of keys container
	 * @note Using C-string to store the keys is inadvisable if the keys themselves contain null characters,
	 * 	since C-strings may not contain null characters excluding their terminator.
	 */
	template<class InputIterator>
	Watchdog(EventLoop& l, unsigned int timeout, InputIterator begin, const InputIterator& end)
	  : timeout(timeout), hasTimedout(false), resetsEnabled(false) {
		reset();
		l.registerIntervalSlot(&Watchdog::check, *this);
		for(; begin != end; begin++)
			associateKey(*begin);
	}
	/**
	 * Associates watchdog with an additional key.
	 * @param key key to associate with
	 */
	void associateKey(const std::string& key);
	/**
	 * Deactivates and destroys the watchdog.
	 */
	virtual ~Watchdog() {} //Since we derived from boost::signals::trackable, the signal-slot connection
							//will be automatically deleted upon destruction.
	/**
	 * Pure virtual function called when a watchdog timeout occurs. Extend class
	 * and implement to define what happens when a watchdog times out.
	 */
	virtual void onTimeout() = 0;

	/**
	 * Resets watchdog timer.
	 */
	void reset();
	/**
	 * Resets watchdog timer. Takes two string parameters (but does nothing with them)
	 * to enable use as Substate slot.
	 */
	void reset(const std::string& k, const std::string& v) { reset(); }
	
	/**
	  * Enables/disables resets from being acknowledged.
	  * @param enable true to enable, false to disable
	  */
	void enableResets(bool enable = true) { resetsEnabled = enable; }
private:
	/**
	 * If the timer was reset more than a timeout duration ago, calls onTimeout().
	 * Is used as a slot connected to an EventLoop interval handler signal.
	 */
	void check();

	const unsigned int timeout; ///< Watchdog timeout in milliseconds.
	timespec lastCheckIn; ///< Time of last check in (reset).
	bool hasTimedout; ///< Used to keep track of previous timeout to prevent triggering more than once per contiguous timeouts.
	bool resetsEnabled;
} DEPRECATED;

}

#endif /* WATCHDOG_H_ */
