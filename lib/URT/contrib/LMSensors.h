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

#ifndef LMSENSORS_H_
#define LMSENSORS_H_

#include <boost/unordered_map.hpp>
#include <string>
#include <stdexcept>

struct sensors_chip_name;

namespace urt {
namespace contrib {

/** Wrapper for lmsensors C library.
  * This class is available as a singleton only due to the construction of the underlying library.
  * All class functions must be called on the singleton, which can be accessed through the static getSingleton().
  *
  * There are two ways to access lmsensors data. getValue() provides direct, synchronous access to name/value pairs. 
  * LMSensorFunctor provides asynchronous, State-oriented access. An LMSensorFunctor can be created through
  * substateUpdater(). The functor then should be associated with an EventLoop's interval signal. Whenever the functor is
  * invoked, it will retrieve the value associated with the given subfeature name and copy it into the given substate.
  * Code to create and use a LMSensorFunctor will look similar to what follows:
  * eventLoop.registerIntervalSlot(LMSensors::getSingleton().substateUpdater("+12V", "_+12V"));.
  */
class LMSensors {
public:
	/** Get a reference to the LMSensors singleton. */
	static LMSensors& getSingleton();
	
	/** Get a value associated with a subfeature name. */
	double getValue(const std::string& name);

	/** Functor that copies value associated with a subfeature name into a substate. */
	class LMSensorFunctor {
	public:
		/** Creates a LMSensorFunctor. For advanced and non-portable use only. Use substateUpdater() instead.
		  * @param subfeature Subfeature number.
		  * @param key Substate key.
		  */
		LMSensorFunctor(int subfeature, const std::string& key) : subfeature(subfeature), key(key) {}
		/** Copy data from lmsensors to substate. */
		void operator()();
	private:
		int subfeature;
		std::string key;
	};
	/** Creates a LMSensorFunctor using given subfeature name.
	  * @param subfeatureName Name of lmsensors subfeatrue associated with value to copy.
	  * @param substateKey Key of substate in which to copy value.
	  */
	LMSensorFunctor substateUpdater(const std::string& subfeatureName, const std::string& substateKey);

	/** Error thrown upon error associated with lmsensors library. */
	struct Error : public std::runtime_error {
		Error(const std::string& msg) : std::runtime_error(msg) {}
	};
private:
	//It's a singleton! No copying or construction allowed by outside forces!
	LMSensors();
	LMSensors(const LMSensors&);
	~LMSensors();

	const sensors_chip_name* chip;
	typedef boost::unordered_map<std::string, int> SubfeatureMap;
	SubfeatureMap subfeatureMap;
};

}
}

#endif
