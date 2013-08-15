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

#ifndef SLOTTEDTIMER_H_
#define SLOTTEDTIMER_H_

#include "Timer.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>

namespace urt {

/** The SlottedTimer provides a signal based on a Timer.
  * The SlottedTimer provided an interval signal independent of a EventLoop's
  * interval signal. It is a child class of Timer and, as such behaves like it,
  * especially in that it must be added to an EventLoop to work properly.
  */
class SlottedTimer : public Timer {
public:
	/** Construct a SlottedTimer.
	  * @param millis Timeout in milliseconds.
	  * @param start If true, timer starts counting immediately after construction.
	  */
	SlottedTimer(int millis, bool start = true) : Timer(millis, start) {}
	
	/**
	 * Registers all slots except for non-static class member functions.
	 * The slot is called at least every timeout duration.
	 *
	 * @param slot pointer to non-member function, functor (object that overloads operator()),
	 * 	or static member function
	 *
	 * @note If the slot is a functor, it cannot be derived from boost::signals::trackable.
	 * @see State::registerSlot
	 */
	void registerSlot(const boost::signal<void ()>::slot_type& slot);
	/**
	 * Registers class member functions with associated object as a slot.
	 * The slot is called at least every timeout duration. The slot will be automatically unregistered if the
	 * object is destroyed.
	 *
	 * @param ptrMemFunc pointer to a member function
	 * @param obj object whose function to call
	 * @tparam T type of object to associate with slot
	 *
	 * @see State::registerSlot
	 */
	template<class T>
	inline void registerSlot(void (T::*ptrMemFunc)(), T& obj) {
		//The reason for casting to boost::signals::trackable and back to T is to enforce
		//the requirement that T be derived from boost::signals::trackable.
		//If you get an error here, it is because you are trying to violate that requirement.
		registerSlot(boost::bind(ptrMemFunc, &static_cast<T&>(static_cast<boost::signals::trackable&>(obj))));
	}
	/**
	 * Convenience function which enables using a boost::weak_ptr (EvtSourcePtr) instead of a reference.
	 * @overload
	 */
	template<class T>
	inline void registerSlot(void (T::*ptrMemFunc)(), boost::weak_ptr<T> ptr) {
		if(boost::shared_ptr<T> p = ptr.lock())
			registerSlot(ptrMemFunc, *p);
	}
private:
	bool onTimeout();
	boost::signal<void ()> signal;
};

}

#endif
