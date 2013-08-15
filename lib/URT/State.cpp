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
 * State.cpp
 *
 *  Created on: Mar 4, 2010
 *      Author: Michael Sechooler
 */

#include "State.h"

namespace urt {


boost::unordered_map<std::string, State::Substate> State::substates;

void State::setSignalOnTouch(const std::string& key, bool v) {
	substates[key].signalOnTouch = v;
}

std::string State::get(const std::string & key)
{
	return substates[key].data;
}

void State::set(const std::string & key, const std::string & value)
{
	if(substates[key].data != value)
	{
		substates[key].data = value;
		(*substates[key].signal)(key, value); //call all of the signals
	} else if(substates[key].signalOnTouch)
		(*substates[key].signal)(key, value); //call all of the signals
}

void State::registerSlot(const std::string& key, const boost::signal<void (const std::string&, const std::string&)>::slot_type& slot)
{
	substates[key].signal->connect(slot);
}


}
