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
 * Socket.cpp
 *
 *  Created on: Mar 1, 2010
 *      Author: Michael Sechooler
 */

#include "Socket.h"

#include <strings.h> //bzero
#include <arpa/inet.h> //inet_addr
#include <poll.h>

namespace urt {

Socket::Socket(int fd) : FDEvtSource(fd), okay(true) {}
Socket::Socket(const char* ipAddress, unsigned short port) throw (SocketException) : okay(true) {
	if(port == 0)
		throw SocketException("Invalid port number");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ipAddress);
	if(addr.sin_addr.s_addr == static_cast<in_addr_t>(-1))
		throw SocketException("Invalid IP address");
	bzero(addr.sin_zero, sizeof(addr.sin_zero));

	fdesc = socket(AF_INET, SOCK_STREAM, 0);

	//enable us to reuse port if we just disconnected
	int options = 1;
	setsockopt(fdesc, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));

	if (connect(fdesc, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw SocketException("Error opening socket");
}

Socket::~Socket() {
	close(fdesc);
}

bool Socket::IsOk()
{
	pollfd pfd = {fdesc, POLLRDHUP, 0};
	return okay && !poll(&pfd, 1, 0); //poll returns non-zero if error
}

void Socket::send(const void* buf, ssize_t size) throw (SocketException)
{
	ssize_t t = write(fdesc, buf, size);
	if(t < 0 || t != size)
	{
		okay = false;
		throw SocketException("Error sending data");
	}
}
size_t Socket::get(void* buf, ssize_t size) throw (SocketException)
{
	ssize_t t = recv(fdesc, buf, size, MSG_WAITALL);
	if(t < 0 || t < size)
	{
		okay = false;
		throw SocketException("Error getting data");
	}
	return t;
}

}
