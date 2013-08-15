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
 * StateSocket.h
 *
 *  Created on: Mar 10, 2010
 *      Author: Michael Sechooler
 */

#ifndef STATESOCKET_H_
#define STATESOCKET_H_

#include "Socket.h"

namespace urt {

/**
 * StateSocket objects permit associated TCP sockets to get and set substates.
 * StateSocket is to StateDevice as attached serial devices are to attached TCP sockets.
 * The StateSocket protocol is similar to the StateDevice protocol save for one or two differences:
 * 	\li The key transmitted by the other host is the one used; no information is prepended.
 * 	\li The key is length-defined, not null-terminated.
 *  \li The StateSocket is not encapsulated in a message-boundary ensuring protocol (like the ARD Protocol).
 *  	Instead, it manages message boundaries itself.
 *
 * The reason for using a length-defined key is to permit the use of null characters in the key.
 * This should be a common occurrence, since it is possible for the prepended application type or
 * UID of a StateDevice to be zero.
 *
 * The StateSocket protocol is defined as follows:<br>
 * <tt>{size: 1 byte}{message_type: 1 byte}{key_size: 1 byte}{key: key_size bytes}{value: size-key_size-2 bytes}</tt><br>
 * If a value is not applicable (for example, when getting a value), the value should be omitted.
 *
 * The following message types are defined:
 * 	\li \c 0x00 remote host is setting a substate
 * 	\li \c 0x01 remote host is getting a substate; server responds with full packet as described above with type \c 0x01
 */
class StateSocket: public Socket {
public:
	/**
	 * Create and open a socket.
	 * @param ipAddress IP address to which to connect in dotted-quad form
	 * @param port port number to which to connect
	 * @throws SocketException thrown when unable to connect to server
	 */
	StateSocket(const char* ipAddress, unsigned short port) throw (SocketException) : Socket(ipAddress, port) {}
	/**
	 * Creates a StateSocket object for an already opened socket given its file descriptor.
	 *
	 * @note The StateSocket takes ownership of the socket and will close it when necessary.
	 */
	StateSocket(int fd) : Socket(fd) {}
	virtual ~StateSocket() {}

	/**
	 * Send a substate key and associated value.
	 * @param key substate key
	 * @param value substate value
	 * @throws SocketException thrown on error
	 */
	void sendSubstate(const std::string& key, const std::string& value) throw (SocketException);

private:
	static unsigned char msg[256];

	//prevent inadvertent use of lower-level get and send calls
	using Socket::get;
	using Socket::send;
	
	bool onActivity();
};

}

#endif /* STATESOCKET_H_ */
