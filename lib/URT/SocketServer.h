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
 * SocketServer.h
 *
 *  Created on: Mar 1, 2010
 *      Author: Michael Sechooler
 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include "FDEvtSource.h"
#include "EventLoop.h"
#include "urtexcept.h"
#include <sys/socket.h>
#include <strings.h>
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <netinet/in.h>

namespace urt {

/**
 * The internal namespace contains all helper functions and classes that, while exposed via
 * public headers, are not intended for general use.
 */
namespace internal {
	template <class SocketType>
	class DefaultOnConnection {
	public:
		bool operator()(EvtSourcePtr<SocketType> socket) { return true; }
	};

	void initializeSocket(int& fdesc, unsigned short port);
	bool IsSocketOk(int fd);
	int acceptSocket(int serverFD) ;
}
/**
 * This class handles low-level client socket communication.
 *
 *  SocketServer listens for connections on a port. When a connection is made, it spawns an appropriate Socket-derived class,
 *  adds the new Socket to the event loop, and optionally calls a functor for additional processing.
 *
 *  @tparam SocketType the type of socket to spawn upon incoming connection; must have a constructor that only takes the file
 *  	descriptor of the new socket
 *  @tparam OnConnection Functor type to call upon connection; must implement <tt>bool operator()(EvtSourcePtr<SocketType> socket)</tt>.
 *  	Return false to veto and drop connection. By default, does nothing.
 */
template <class SocketType, class OnConnection = internal::DefaultOnConnection<SocketType> >
class SocketServer : public urt::FDEvtSource {
public:
	/**
	 * Create a SocketServer using a default-constructed OnConnection object.
	 * @param port port number on which to listen
	 * @param l EventLoop to which to add new connections
	 */
	SocketServer(unsigned short port, EventLoop* l) : evtloop(l) {
		internal::initializeSocket(fdesc, port);
	}
	/**
	 * Create a SocketServer using a non-default OnConnection object.
	 * @param port port number on which to listen
	 * @param l EventLoop to which to add new connections
	 * @param o copy of functor to call upon each connection
	 */
	SocketServer(unsigned short port, EventLoop* l, const OnConnection& o) : evtloop(l), onConnection(o) {
		internal::initializeSocket(fdesc, port);
	}
	~SocketServer() {}

	/**
	 * Check if SocketServer is alive and okay
	 * @return true if okay
	 */
	bool IsOk() {
		return internal::IsSocketOk(fdesc);
	}

	/**
	 * Called upon activity on the server socket.
	 * @note This function is not virtual; it automatically handles all incoming connections. To influence
	 *	the connection process, use a non-default OnConnection functor object.
	 */
	bool onActivity() {
		if(IsOk()) {
			int fd = internal::acceptSocket(fdesc);

			if(fd < 0)
				return true; //while there was an error, let's assume that the server socket is still fine

			LockedEvtSourcePtr<SocketType> ptr(new SocketType(fd));
			if(onConnection(ptr))
				evtloop->add(ptr);

			return true;
		}
		return false;
	}

private:
	EventLoop* evtloop;
	OnConnection onConnection;
};

}

#endif /* SOCKETSERVER_H_ */
