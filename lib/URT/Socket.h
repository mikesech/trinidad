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
 * Socket.h
 *
 *  Created on: Mar 1, 2010
 *      Author: Michael Sechooler
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include "FDEvtSource.h"
#include "urtexcept.h"

#include <netinet/in.h>

namespace urt {

/**
 * This class handles low-level client socket communication.
 *
 *  Socket implements low-level client socket communication. While writing is synchronous, reading is not. Whenever there is
 *  data available to be read on the port, an event will be generated and activity() will be called. Socket does not implement
 *  activity() and is as such an abstract class.
 *
 *  Generally, implementing some form of event-driven socket I/O involves the following:
 *  <ol>
 *  <li>Extend this class</li>
 *  <li>Implement activity().
 *		<ul><li>Utilize normal system calls to read from the inherited file descriptor \c fdesc in activity().
 *		If data is still available after the handler is complete, another event will be generated.</li></ul>
 *  </li>
 *  </ol>
 */
class Socket: public urt::FDEvtSource {
public:
	/**
	 * Create and open a socket.
	 * @param ipAddress IP address to which to connect in dotted-quad form
	 * @param port port number to which to connect
	 * @throws SocketException thrown when unable to connect to server
	 */
	Socket(const char* ipAddress, unsigned short port) throw (SocketException);
	/**
	 * Creates a Socket object for an already opened socket given its file descriptor.
	 *
	 * @note The Socket takes ownership of the socket and will close it when necessary.
	 */
	Socket(int fd);
	/**
	 * Closes and deletes socket.
	 */
	virtual ~Socket();

	/**
	 * Send data to server.
	 * @param buf pointer to data to send
	 * @param size size of buffer to send in bytes
	 * @throws SocketException thrown when error sending data
	 */
	void send(const void* buf, ssize_t size) throw (SocketException);
	/**
	 * Receive data from server.
	 * @param buf pointer to buffer in which to save data
	 * @param size size of buffer and size of data to receive (blocks until buffer is filled)
	 * @return number of bytes received and stored (should be equal to size of buffer)
	 * @throws SocketException thrown when error receiving data
	 */
	size_t get(void* buf, ssize_t size) throw (SocketException);
	/**
	 * Determines if socket is alive and okay.
	 * @return true if okay
	 */
	bool IsOk();

private:
	bool okay;
	sockaddr_in addr;
};

}

#endif /* SOCKET_H_ */
