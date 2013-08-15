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
 * StateDevice.h
 *
 *  Created on: Mar 4, 2010
 *      Author: Michael Sechooler
 */

#ifndef STATEDEVICE_H_
#define STATEDEVICE_H_

#include "ArdPort.h"

namespace urt {

/**
 * Connects the concept behind both the ARD protocol and the State class. StateDevices
 * are attached Arduino devices that understand the concept of State. When they transmit
 * data, it is always in key-value pair form. The server simply has to then commit that
 * pair to the State. Since this process is the same for any kind of sensor device,
 * only one type of class, StateDevice, is needed. There is no need to extend this class
 * for a particular device provided it uses the StateDevice protocol. (Unlike its ancestors
 * StateDevice is, in fact, a concrete class and contains no virtual functions or destructor.)
 *
 * @note The key used by State is not the same as the key sent by the device. Instead,
 * 	the application type and UID is prepended to the sent state. This opens up a wide
 * 	range of options. Having multiple devices with the same type and UID creates redundancy;
 * 	the State holds the most recent data, which will be refreshed as long as one device still works.
 * 	Devices of the same type but different UID enables using multiple devices while keeping their data
 * 	separate.
 *
 * The protocol defines an outgoing message of type 0x00 with no payload to indicate that
 * the device should update all possible states immediately. This is implemented in poll().
 * In general, this function would be connected as a slot to the event loop's interval signal,
 * essentially regularly polling the devices each timeout period.
 *
 * The protocol also permits the device to register a substate for push notification. The server will
 * automatically notify the device if the substate changes as if the device requested its status.
 * The device should register all desired substates immediately after handshaking with the server
 * (handling a message of 0xFF, as defined in the ARD protocol); it should not register any other time.
 * Registering a substate multiple times will causes the server to push the update an equal number of times.
 * Whenever a handshake is conducted, all prior regisrations are void.
 *
 * The different message options for device to server communication are as follows:
 * <table>
 * <tr>	<td>\b Type</td>	<td>\b Message \b Contents</td>	<td>\b Comment</td></tr>
 * <tr>	<td>0x00</td>		<td>(key length: 1 byte)(key)(value)</td>	<td>device is setting state</td></tr>
 * <tr>	<td>0x01</td>		<td>(key length: 1 byte)(key)</td>		<td>device is requesting state</td></tr>
 * <tr>	<td>0x02</td>		<td>(key length: 1 byte)(key)</td>		<td>device is registering substate for push notification</td></tr>
 * </table>
 *
 * The different message options for server to device communication are as follows:
 * <table>
 * <tr>	<td>\b Type</td>	<td>\b Message \b Contents</td>	<td>\b Comment</td></tr>
 * <tr>	<td>0x00</td>		<td>empty</td>								<td>polling; device should update server of all possible substates</td></tr>
 * <tr>	<td>0x01</td>		<td>(key length: 1 byte)(key)(value)</td>	<td>response to device request</td></tr>
 * </table>
 */
class StateDevice : public ArdPort {
public:
	/**
	 * Constructs StateDevice.
	 * @param dev path to serial port device file
	 */
	StateDevice(const char* dev) : ArdPort(dev) {}

	/**
	 * Transmits message type 0x00 with no payload to to indicate that
	 * the device should update all possible states immediately.
	 * @throw SerialException thrown on error
	 */
	void poll() throw (SerialException);

	/**
	 * Sends a Substate to associated device consisting of key, value pair.
	 *
	 * @note The key, value pair does not need to be in the global State class. However, sending
	 * 	information not consistent with the global State is bad form and practice and is highly
	 * 	discouraged.
	 * @param key key part of key-value pair to send
	 * @param value value part of key-value pair to send
	 * @param removeIDs if true, will remove first two characters of key if they match the device's application type and UID
	 */
	void sendSubstate(const std::string& key, const std::string& value, bool removeIDs);
	/**
	  * Sends a Substate to associated device consisting of key, value pair, first removing the device's application type and UID
	  * from the key, if applicable.
	  *
	  * @note This function is equivalent to calling sendSubstate(key, value, true). It is provided as an overload and not by using
	  *   a default parameter to permit its use as a Substate slot.
	  */
	void sendSubstate(const std::string& key, const std::string& value) { sendSubstate(key, value, true); }
	/**
	 * Sends a Substate from the global State class to associated device.
	 * @overload
	 */
	void sendSubstate(const std::string& key, bool removeIDs = true);

private:
	//prevent inadvertent use of lower-level I/O calls
	using ArdPort::getDatagram;
	using ArdPort::sendDatagram;
	
	bool onActivity();
};

}

#endif /* STATEDEVICE_H_ */
