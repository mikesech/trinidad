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
 * ArdPort.h
 *
 *  Created on: Mar 3, 2010
 *      Author: Michael Sechooler
 */

#ifndef ARDPORT_H_
#define ARDPORT_H_

#include "SerialPort.h"
#include "urtexcept.h"
#include <string>

namespace urt {

/**
 * The ArdPort class implements URT's ARD protocol over a serial port. The ARD protocol
 * provides a standard way to interface with connected Arduino (and other) devices.
 * It defines a standard datagram and handshaking process, enabling client code
 * to automatically detect the attached device.
 *
 * @note ArdPort is an abstract class. It does not implement onActivity(). To use ArdPort, extend the class,
 * 	implement onActivity(), and utilize getDatagram() to retrieve the data.
 *
 * A datagram is defined as follows:<br>
 * <tt>{length of remaining datagram: 1 byte}{message type: 1 byte}{message}{bitwise inverse of first byte: 1 byte}</tt>
 *
 * The first byte is the number of bytes in the datagram following the first byte. The last byte is the bitwise inverse of the first.
 * The overhead for each datagram is 3 bytes; the maximum size of each message is 253 bytes.
 *
 * @note The first byte cannot be less than two. If a byte that would otherwise mark the start of a datagram is either a 0 or 1,
 * 	the Arduino or similar device will discard it (note: this is not true for the server). By flooding the connection with null
 *  characters, it is possible to reset the connection and recover from any synchronization errors.
 *  This technique is utilized by the server at the handshake to flush the connection.
 *
 * The message type 0xFF is reserved. It is currently used in the handshaking process, which is as follows: <br>
 * 		Computer: <tt>0x02 0xFF 0xFD</tt> -- a datagram of type 0xFF with no message. <br>
 * 		Device: <tt>0x04 0xFF (application type; 1 byte) (unique identifier; 1 byte) 0xFB</tt> -- a datagram of type 0xFF
 * 			with the application type and UID as the message.
 *
 * @note Ideally, the UID would be unique for each device of its application type so that the application type and UID combined would
 * 		uniquely identify each device. However, since programming each device with a different UID would be time-consuming, it is
 * 		possible -- in fact, suggested -- to just use 0 as the UID.
 *
 * Additionally, it defines the attributes of the RS-232 link:
 * \li 2400 baud
 * \li 1 stop bits
 * \li 8 data bits
 * \li no parity
 */
class ArdPort: public urt::SerialPort {
public:
	/**
	 * Initializes ArdPort device on the given special device file.
	 * @param dev path to serial port device file
	 * @throws SerialException thrown on error opening port or handshaking
	 */
	ArdPort(const char* dev) throw (SerialException);
	virtual ~ArdPort();

	/**
	 * Receives a datagram.
	 * @param buf reference to std::string in which to store the datagram
	 * @return the datagram type
	 * @throws SerialException thrown on error
	 */
	unsigned char getDatagram(std::string& buf) throw (SerialException);
	/**
	 * Sends a datagram.
	 * @param type type of message
	 * @param datagram the datagram
	 * @throws SerialException thrown on error
	 */
	void sendDatagram(unsigned char type, const std::string& datagram) throw (SerialException) { sendDatagram(type, datagram.c_str(), datagram.size()); }
	/**
	 * Sends a datagram.
	 * @overload
	 * @param type of message
	 * @param datagram the datagram
	 * @param len length of datagram
	 * @throws SerialException thrown on error
	 */
	void sendDatagram(unsigned char type, const char* datagram, unsigned char len) throw (SerialException);
	/**
	 * Get application type reported by device.
	 * @return application type
	 */
    unsigned char getAppType() const { return appType; }
    /**
	 * Get UID (unique identifier) reported by device.
	 * @return UID
	 */
    unsigned char getUid() const { return uid; }

private:
	unsigned char appType;
	unsigned char uid;

	//prevent inadvertent use of lower-level get and send calls
	using SerialPort::get;
	using SerialPort::send;
};

}

#endif /* ARDPORT_H_ */
