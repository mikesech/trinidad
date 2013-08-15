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
 * StateSocket.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: Michael Sechooler
 */

#include "StateSocket.h"
#include "State.h"

namespace urt {

unsigned char StateSocket::msg[256];

void StateSocket::sendSubstate(const std::string& key, const std::string& value) throw (SocketException) {
	std::string returnMsg(1, key.size() + value.size() + 2);
	returnMsg += 0x01;
	returnMsg += static_cast<unsigned char>(key.size());
	returnMsg += key;
	returnMsg += value;
	send(returnMsg.c_str(), returnMsg.size());
}

bool StateSocket::onActivity() {
	try {
		unsigned char msgSize;
		get(&msgSize, sizeof(msgSize));

		get(msg, msgSize);

		switch(msg[0]) {//message type
			case 0x00: {
				//key size = msg[1], key = msg[2], value = msg[msg[1] + 2]
				State::set(std::string(reinterpret_cast<char*>(&msg[2]), msg[1]), std::string(reinterpret_cast<char*>(&msg[msg[1] + 2]), msgSize - msg[1] - 2));
				break;
			}
			case 0x01: {
				std::string key(reinterpret_cast<char*>(&msg[2]), msgSize - 2);
				sendSubstate(key, State::get(key));
				break;
			}
		}
		return IsOk();
	} catch (...) { return false; }
}

}
