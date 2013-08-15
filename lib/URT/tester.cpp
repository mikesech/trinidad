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
 * tester.cpp
 *
 *  Created on: Feb 28, 2010
 *      Author: Michael Sechooler
 */

#include "EventLoop.h"
#include "Socket.h"
#include "SocketServer.h"
#include "State.h"
#include "StateDevice.h"
#include "StateSocket.h"
#include "ExternalProgram.h"
#include "Watchdog.h"
#include "Log.h"

using namespace urt;

#include <iostream>
#include <string>
#include <list>
#include <cctype>
#include <iterator>

using std::cout;
using std::endl;

void printValue(const std::string& key, const std::string& value) {
	cout<<value<<endl;
}

struct PrintSocket : public Socket {
	PrintSocket(int fd) : Socket(fd) {}
	bool onActivity() {
		if(!IsOk()) {
			std::cerr<<"PrintSocket dying!"<<endl;
			return false;
		}
		char b;
		read(fdesc, &b, 1);
		std::cout<<b;
		return true;
	}
};

struct MyWatchdog : public Watchdog {
	MyWatchdog(EventLoop& l, const std::string& key) : Watchdog(l, 10000, key) {}
	template<class InputIterator>
	MyWatchdog(EventLoop& l, InputIterator begin, const InputIterator& end) : Watchdog(l, 10000, begin,end) {}
	void onTimeout() {
		std::cerr<<"*** Oh no! Watchdog timed out."<<std::endl;
	}
};

void printTimestamp() {
	time_t t = time(NULL);
	std::cout<<"\n**** "<<ctime(&t)<<std::endl;
}

int main(int argc, char* argv[])
{
	Log::message("Welcome!");
	EventLoop loop(500);
	//loop.registerIntervalSlot(&printTimestamp);
	State::registerSlot("key", printValue);
	loop.add(new SocketServer<StateSocket>(4444,&loop));
	//loop.add(new StateDevice("/dev/pts/5"));
	//loop.add(ExternalProgram<StateSocket>::Create(false, "nc", "-clp", "4445", (const char*)NULL));
	MyWatchdog dog(loop, "key");
	loop.run();

	Log::error("No event handlers exist");
	return 0;
}
