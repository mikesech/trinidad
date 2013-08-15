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

#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <vector>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>

#include <poll.h>
#include <ctime>
//struct pollfd;
/** Convenience definition. boost::weak_ptr is used to reference a watched FDEvtSource from outside */
#define EvtSourcePtr boost::weak_ptr
/** Convenience definition. boost::weak_ptr is used to track a watched FDEvtSource from inside */
#define LockedEvtSourcePtr boost::shared_ptr

namespace urt
{
	class FDEvtSource;
	/** EventLoop forms the backbone of the URT event-driven system.
	 *  While running, it will continuously check for any activity on associated
	 *  event sources (presently only FDEvtSource and derived classes) and call the
	 *  appropriate handlers.
	 *
	 *  @note The event loop takes ownership of any attached event sources; it will delete them when appropriate. Therefore,
	 *  \b never create a source on the stack. Always use the \c new operator.
	 */
	class EventLoop
	{
		public:
			/**
			 * Create an EventLoop.
			 * @param timeout Approximately every \c timeout milliseconds, the event handler signal will be activated.
			 */
			EventLoop(int timeout = 10000);
			//~EventLoop();

			void run();
			bool add(const boost::shared_ptr<FDEvtSource>& fdsource);
			/** Add an FDEvtSource to event loop.
			 * The EventLoop takes ownership of the FDEvtSource; it will delete the source when appropriate. As a result,
			 * the FDEvtSource <b>must not be created on the stack</b> but rather with the \c new operator.
			 * A common way to invoke add() is <tt>add(new FDEvtSource())</tt>, creating and adding the FDEvtSource in one step.
			 *
			 * The function returns a boost::weak_ptr (aliased as EvtSourcePtr) to access the supplied parameter. An attempt
			 * to lock the weak pointer will fail if the event loop has already deleted the object.
			 * @note If you do not want the event loop to delete the object, add it with add(const boost::shared_ptr<FDEvtSource>&),
			 * 	using a boost::shared_ptr. In either case, the object will only be delete if there are no other <tt>shared_ptr</tt>s
			 * pointing to the FDEvtSource
			 *
			 * @tparam T type of FDEvtSource to add
			 * @param fdsource pointer to FDEvtSource to add
			 * @return weak pointer to \c fdsource
			 */
			template<class T>
			EvtSourcePtr<T> add(T* fdsource)
			{
				boost::shared_ptr<T> ptr(fdsource);
				if(add(ptr))
					return EvtSourcePtr<T>(ptr);
				else
					return EvtSourcePtr<T>();
			}
			bool remove(FDEvtSource* fdsource);
			bool has(FDEvtSource* fdsource);

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
			void registerIntervalSlot(const boost::signal<void ()>::slot_type& slot);
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
			inline void registerIntervalSlot(void (T::*ptrMemFunc)(), T& obj) {
				//The reason for casting to boost::signals::trackable and back to T is to enforce
				//the requirement that T be derived from boost::signals::trackable.
				//If you get an error here, it is because you are trying to violate that requirement.
				registerIntervalSlot(boost::bind(ptrMemFunc, &static_cast<T&>(static_cast<boost::signals::trackable&>(obj))));
			}
			/**
			 * Convenience function which enables using a boost::weak_ptr (EvtSourcePtr) instead of a reference.
			 * @overload
			 */
			template<class T>
			inline void registerIntervalSlot(void (T::*ptrMemFunc)(), boost::weak_ptr<T> ptr) {
				if(boost::shared_ptr<T> p = ptr.lock())
					registerIntervalSlot(ptrMemFunc, *p);
			}

		private:
			const int timeout; //in milliseconds
			size_t indexFDSource(FDEvtSource* fdsource);

			timespec timeLastInterval; ///< Time last interval handler was executed
			int adjustedTimeout; ///< Adjusted so that (current time) + adjustedTimeout - timeLastInterval = timeout
			std::vector<boost::shared_ptr<FDEvtSource> > fdsources;
			std::vector<pollfd> fds;
			std::vector<size_t> deleteQueue;
			std::queue<boost::shared_ptr<FDEvtSource> > addQueue;
			boost::signal<void ()> intervalSignal;
			bool running;
	};
}

#endif /* EVENTLOOP_H_ */
