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
 * State.h
 *
 *  Created on: Mar 4, 2010
 *      Author: Michael Sechooler
 */

#ifndef STATE_H_
#define STATE_H_

#include <map>
#include <string>
#include <boost/signal.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp> //hash map
#include <boost/lexical_cast.hpp>

namespace urt {

/**
 * The State static class keeps track of all the different states of the machine at any given time.
 * It does so dynamically, using a map to store keyed data. The general idea is that
 * the event handlers for sensors will take incoming data and store them into a global state object.
 * The TCP socket event handlers will get that data and report it back to the socket when polled.
 * In this way, the State object is really the only common thread in the URT system (besides possibly
 * the EventLoop).
 *
 * @note State is a so-called static class. It is not possible to instantiate objects of the State class.
 * Instead, all member functions and variables are declared static. In a sense, this makes State a global
 * class object. There is only one State, and it persists throughout execution.
 *
 * A Substate consists of a string key and a string value. Numerical value types (int, double, etc.) were
 * considered but rejected due to the additional complexity and the difficultly in transmitting numbers
 * across different platforms with possible differences in endianness. Only one value exists per state;
 * attempting to insert/set a key-value pair will replace a previous key-value pair that had the same key.
 *
 * The State class also enables the use of signals, which enables client code to tell the system to trigger
 * a function (called a slot) when there is a change to State. Generally speaking, this won't be used too often, since data
 * is generally sent out only on the request of the recipient. However, this ability may be useful for things
 * which cannot wait: movement instructions, for example. Note that signals are executed synchronously; they will
 * be invoked immediately after changing the State and the function used to change the state will not return until
 * they are finished. It is important, therefore, to not get stuck in a signal-invoking loop which never
 * returns control back to the handler and event loop; this could happen if the client code invoked on a signal
 * alters the state again, triggering another signal. (Pro tip: use signals sparingly and only when necessary).
 */
class State {
public:
	/**
	 * Set whether a substate's signal will be triggered if it is just touched (i.e., set to its already set value) and not
	 * 	changed.
	 * @param key substate's key
	 * @param v if true, \b all slots associated with the substate signal will fire on touches
	 */
	static void setSignalOnTouch(const std::string& key, bool v);

	/**
	 * Set a substate.
	 * @param key substate's key
	 * @param value substate's value
	 */
	static void set(const std::string& key, const std::string& value);
	/**
	 * Set a substate, converting argument to string.
	 * @tparam T type of value
	 * @param key substate's key
	 * @param value substate's value
	 * @throws boost::bad_lexical_cast thrown if not convertible to string form
	 */
	template<typename T>
	inline static void set(const std::string& key, const T& value) {
		set(key, boost::lexical_cast<std::string>(value));
	}
	/**
	 * Get a substate.
	 * @param key substate's key
	 * @return substate's value; empty string if not set
	 */
	static std::string get(const std::string& key);
	/**
	 * Get a substate as a particular type.
	 * @tparam T type to return
	 * @param key substate's key
	 * @return substate's value
	 * @throws boost::bad_lexical_cast thrown if not convertible or if unset
	 */
	template<typename T>
	inline static T getAs(const std::string& key) throw (boost::bad_lexical_cast) {
		return boost::lexical_cast<T>(get(key));
	}
	/**
	 * Get a substate as a particular type. Returns a default value if unset.
	 * @tparam T type to return
	 * @param key substate's key
	 * @param unset default value to return if unset
	 * @return substate's value
	 * @throws boost::bad_lexical_cast thrown if not convertible
	 */
	template<typename T>
	inline static T getAs(const std::string& key, T unset) throw (boost::bad_lexical_cast) {
		const std::string t = get(key);
		if(t.empty())
			return unset;
		else
			return boost::lexical_cast<T>(t);
	}
	/**
	 * Registers all slots except for non-static class member functions.
	 * The slot is called whenever the state for the given key <i>changes</i>
	 * (not necessarily when it is set, unless substate has been instructed via setSignalOnTouch()).
	 * The slot must take two const std::string references as parameters. The first is the key changed; the second,
	 * the new value.
	 *
	 * @param key key to associate slot
	 * @param slot pointer to non-member function, functor (object that overloads operator()(const std::string&, const std::string&),
	 * 	or static member function
	 *
	 * @note If the slot is a functor, it cannot be derived from boost::signals::trackable.
	 *
	 * @code
	 * void announce(const std::string& key, const std::string& value) {
	 * 		std::cout<<"Something happened!\n";
	 * }
	 * struct Functor {
	 * 		void operator()(const std::string& key, const std::string& value) {
	 * 			std::cout<<"Look at me! I'm a functor\n";
	 * 		}
	 * };
	 * int main() {
	 * 		State::registerSlot("hello", &announce);
	 *		State::registerSlot("hello", Functor());
	 * }
	 * @endcode
	 */
	static void registerSlot(const std::string& key, const boost::signal<void (const std::string&, const std::string&)>::slot_type& slot);
	/**
	 * Registers class member functions with associated object as a slot.
	 * The slot is called whenever the state for the given key <i>changes</i>
	 * (not necessarily when it is set, unless substate has been instructed via setSignalOnTouch()).
	 * The slot will be automatically unregistered if the object is destroyed.
	 * The slot must take two const std::string references as parameters. The first is the key changed; the second,
	 * the new value.
	 *
	 * @param key key to associate slot
	 * @param ptrMemFunc pointer to a member function
	 * @param obj object whose function to call
	 * @tparam T type of object to associate with slot
	 *
	 * @note The object must be derived from \c boost::signals::trackable, which is defined
	 * 		in <boost/signals/trackable.hpp>. Note that \c FDEvtSource itself derives from it.
	 *
	 * @code
	 * class Print : public boost::signals::trackable {
	 * public:
	 * 		Print(const std::string& s) : p(s) {}
	 * 		void print(const std::string& key, const std::string& value) { std::cout<<p<<endl; }
	 * private:
	 * 		std::string p;
	 * };
	 * int main() {
	 * 		Print p("Goodbye!");
	 * 		State::registerSlot("hello", &Print::print, p);
	 * }
	 * @endcode
	 */
	template<class T>
	inline static void registerSlot(const std::string& key, void (T::*ptrMemFunc)(const std::string&, const std::string&), T& obj) {
		//The reason for casting to boost::signals::trackable and back to T is to enforce
		//the requirement that T be derived from boost::signals::trackable.
		//If you get an error here, it is because you are trying to violate that requirement.
		registerSlot(key, boost::bind(ptrMemFunc, &static_cast<T&>(static_cast<boost::signals::trackable&>(obj)), _1, _2));
	}
	/**
	 * Convenience function which enables using a boost::weak_ptr (EvtSourcePtr) instead of a reference.
	 * @overload
	 */
	template<class T>
	inline static void registerSlot(const std::string& key, void (T::*ptrMemFunc)(const std::string&, const std::string&), boost::weak_ptr<T> ptr) {
		//This is okay because, provided the object is derived from boost::signals::trackable (as is enforced by
		//the above function), the signal-slot connection will be deleted when the object is deleted. Since the slot
		//is associated with a reference and not a shared_ptr, registering it will not affect when it is deleted.
		if(boost::shared_ptr<T> p = ptr.lock())
			registerSlot(key, ptrMemFunc, *p);
	}

private:
	State() {}
	/**
	 * Holds substate data. Only used within State class and is marked private.
	 */
	struct Substate {
		std::string data;
		/* All STL containers (like std::map) require their containees to be copyable.
		 * However, boost::signal<> is not copyable, so we must store a pointer.
		 * But, Substate will be copied. To work around, we use a shared_ptr and create
		 * the object in the zero-parameter constructor. We'll use the default copy constructor,
		 * which will just copy the pointer. When all copies of a Substate are deleted,
		 * there will be no more shared_ptrs to the signal, and it will be deleted.
		 */
		boost::shared_ptr<boost::signal<void (const std::string&, const std::string&)> > signal;
		bool signalOnTouch;

		Substate() : signal(new boost::signal<void (const std::string&, const std::string&)>), signalOnTouch(false) { }
	};
	static boost::unordered_map<std::string, Substate> substates; ///< Map of all substate data.
};

}

#endif /* STATE_H_ */
