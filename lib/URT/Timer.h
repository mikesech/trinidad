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

#ifndef TIMER_H_
#define TIMER_H_

#include "FDEvtSource.h"
#include "urtexcept.h"
#include <signal.h>

namespace urt {

/** The Timer class provides a fine-grained and independent timer.
  * Normally, the only way to call code on a time basis is to use an
  * EventLoop's interval signal. However, while this method does not
  * provide multiple intervals. Generally, to operate at other intevals,
  * one would have to set the EventLoop interval to a low setting and
  * call code which checks to see if a certain time has elasped. However,
  * this was not fine-grained (the granularity restricted to multiples of
  * the interval signal) and required unwieldy and repetitive code.
  *
  * The Timer class acts as a relatively normal FDEvtSource. It keeps track
  * of time independently of the EventLoop and enables fine-grained control.
  * In fact, it is possible that the EventLoop interval signal may become
  * deprecated in favor of Timer in the future.
  *
  * @note Timer behaves differently from the EventLoop interval signal in that
  *	it schedules the next timer event based on when the last timer event
  *	occured, not when it was services. For example, a timer with interval
  *	10 ms would, of course, trigger an event at 10 ms. If it is serviced
  *	(i.e., the EventLoop calls onActivity()) at 15ms, the next event will
  *	still occur at 20 ms.
  *
  * @attention Timer does not allow events to accrue; the timer inhibits events
  *	(although it still counts time) between the event occurance and when it
  *	is serviced. If enough time lapses between the start of the timer and the start
  *	of the EventLoop, the timer will trigger at said start.
  */
class Timer : public FDEvtSource {
public:
	/** Creates a Timer with a given interval.
	  * The timer starts immediately. If enough time lapses between start and
	  * the start of the EventLoop, the timer will trigger at said start.
	  * @param millis Timeout in milliseconds.
	  * @param start If true, timer is started immediately upon creation.
	  * @throw TimerException Thrown if unable to create necessary OS primitives.
	  */
	Timer(unsigned int millis, bool start = true) throw (TimerException);
	~Timer();
	
	/** Stops the timer. */
	void stop();
	/** Starts and resets the timer. */
	void start();
	/** Resets and starts the timer (same as start()). */
	void reset() { start(); }
protected:
	/** Called when running and the timer has expired.
	  * The timer resets itself immediately upon expiration, which can
	  * and is probably eariler than onTimeout is called.
	  * @return False to remove Timer from EventLoop.
	  */
	virtual bool onTimeout() = 0;
private:
	bool onActivity();
	static void timerThread(sigval_t obj);
	
	int writePipe; ///< fd for send end of pipe (used by Timer's thread)
	struct itimerspec m_its;
	timer_t m_timerid;
};

}

#endif
