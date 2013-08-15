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
 * ArdPort.cpp
 *
 *  Created on: Mar 3, 2010
 *      Author: Michael Sechooler
 */

#include "ArdPort.h"
#include <cstring> //for std::memset
#include "Log.h"
#include <poll.h>

using namespace urt;

static const tcflag_t BAUD_RATES[] = {B2400, B19200};
static const size_t NUM_BAUD_RATES = sizeof(BAUD_RATES)/sizeof(*BAUD_RATES);
static const int HANDSHAKE_BYTE_TIMEOUT = 5; //deciseconds

ArdPort::ArdPort(const char *dev) throw (SerialException) : SerialPort(dev, BAUD_RATES[0], false, true, CS8) {
	setTimeout(HANDSHAKE_BYTE_TIMEOUT);
	for(const tcflag_t* baud = &BAUD_RATES[0]; baud < BAUD_RATES + NUM_BAUD_RATES; baud++) {
		//set baud rate
		setSpeed(*baud);
		
		//send handshaking sequence
		/* Flush connection with 255 null bytes */ {
			char flush[255];
			std::memset(flush, 0, 255);
			send(flush, 255);
		}
		flushInput();
		sendDatagram(0xFF, NULL, 0); //no message
		
		struct pollfd pfd = {fdesc, POLLIN, 0};
		if(poll(&pfd, 1, HANDSHAKE_BYTE_TIMEOUT * 100) <= 0) continue;
		try {
			unsigned char handshake[5];
			get(reinterpret_cast<char*>(handshake), 5);
			
			if(handshake[0] == 4 && handshake[1] == 0xFF && handshake[4] == (unsigned char)~4) {
				appType = handshake[2];
				uid = handshake[3];
				setTimeout(0);
				return;
			}
		} catch(...) {}
	}
	
	throw SerialException("ARD handshaking error");
}

ArdPort::~ArdPort() {
	Log::msg<<"ArdPort device deleted of type 0x"<<std::hex<<(unsigned int)appType<<" and UID 0x"<<std::hex<<(unsigned int)uid<<"."<<std::endl;
}

unsigned char ArdPort::getDatagram(std::string& buf) throw (SerialException)
{
	char buffer[256];
	unsigned char size;
	get(reinterpret_cast<char*>(&size), 1);

	get(buffer, size);

	unsigned char type = buffer[0];
	unsigned char size2 = ~buffer[size - 1];
	
	if(size != size2)
		throw SerialException("Invalid ARD datagram");

	buf.assign(buffer + 1, size - 2);
	return type;
}

void ArdPort::sendDatagram(unsigned char type, const char *datagram, unsigned char len) throw (SerialException)
{
	if(len > 253) //too large for next addition
		throw SerialException("ARD datagram too large to send");
	unsigned char size = len + 2; //overhead is 2 bytes (not including first size byte)
	unsigned char size2 = ~size;
	send(reinterpret_cast<char*>(&size), 1);

	send(reinterpret_cast<char*>(&type), 1);
	send(datagram,len);
	send(reinterpret_cast<char*>(&size2), 1);
}




