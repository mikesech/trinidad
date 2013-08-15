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

#include "SocketServer.h"

namespace urt {
void internal::initializeSocket(int& fdesc, unsigned short port) {
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	bzero(serverAddr.sin_zero, sizeof(serverAddr.sin_zero));

	fdesc = socket(AF_INET, SOCK_STREAM, 0);
	int options = 1;
	//enable us to reuse port if we just disconnected
	setsockopt(fdesc, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));

	if (fdesc < 0 || bind(fdesc, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		throw SocketException("Unable to open port");

	if(listen(fdesc,5)) //if error
		throw SocketException("Unable to listen for connections");
}

bool internal::IsSocketOk(int fd) {
	pollfd pfd = {fd, POLLRDHUP, 0};
	return !poll(&pfd, 1, 0); //poll returns non-zero if error
}

int internal::acceptSocket(int serverFD) {
	in_addr_t clientAddr;
	socklen_t clilen = sizeof(clientAddr);
	return accept(serverFD, (sockaddr*) &clientAddr, &clilen);
}
}
