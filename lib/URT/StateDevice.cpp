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
 * StateDevice.cpp
 *
 *  Created on: Mar 4, 2010
 *      Author: Michael Sechooler
 */

#include "StateDevice.h"
#include "State.h"
#include <string>

using namespace urt;

bool StateDevice::onActivity() {
	try {
		std::string data;
		unsigned char type = getDatagram(data);
		switch(type) {
			case 0x00: {
				size_t keySize = data[0];
				std::string key(1, getAppType());
				key += getUid();
				key += data.substr(1, keySize);
				State::set(key, data.substr(1 + keySize));
				break;
			}
			case 0x01: {
				std::string key(1, getAppType());
				key += getUid();
				key += data.substr(1);

				data += State::get(key);
				sendDatagram(0x01, data);
				break;
			}
			case 0x02: {
				std::string key(1, getAppType());
				key += getUid();
				key += data.substr(1);
			  
				State::registerSlot(key, &StateDevice::sendSubstate, *this);
				break;
			}
			default:
				return false;
		}
	} catch (...) {
		return false;
	}
	return true;
}
void StateDevice::sendSubstate(const std::string& key, const std::string& value, bool removeIDs) {
	std::string data;
	if(removeIDs && key[0] == getAppType() && key[1] == getUid()) {
		data = static_cast<unsigned char>(key.size() - 2);
		data += key.substr(2);
	} else {
		data = static_cast<unsigned char>(key.size());
		data += key;
	}
	data += value;
	sendDatagram(0x01, data);
}
void StateDevice::sendSubstate(const std::string& key, bool removeIDs) {
	sendSubstate(key, State::get(key), removeIDs);
}

void StateDevice::poll() throw (SerialException) {
try {
	sendDatagram(0x00, std::string());
} catch(...) {} //ignore for now; hopefully, it will be picked up in the event loop
}
