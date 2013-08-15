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

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <termios.h>
#include <stdexcept>
#include "urtexcept.h"
#include "FDEvtSource.h"

namespace urt
{
	/** Describes the different serial parity options. */
	enum Parity { NONE, ODD, EVEN };

	/** This class handles low-level serial communication.
	 *
	 *  SerialPort implements low-level serial communication. While writing is synchronous, reading is not. Whenever there is
	 *  data available to be read on the port, an event will be generated and onActivity() will be called. SerialPort does not
	 *  implement activity()and is as such an abstract class.
	 *
	 *  Generally, implementing some form of event-driven serial I/O involves the following:
	 *  <ol>
	 *  <li>Extend this class</li>
	 *  <li>Implement activity().
	 *		<ul><li>Utilize normal system calls to read from the inherited file descriptor \c fdesc in activity().
	 *		If data is still available after the handler is complete, another event will be generated.</li></ul>
	 *  </li>
	 *  </ol>
	 */
	class SerialPort : public FDEvtSource
	{
		public:
			SerialPort(const char* dev, tcflag_t baud, bool echo = false, bool block = true,
					tcflag_t dataBits = CS8, Parity parity = NONE, bool twoBits = false)
					throw (SerialException);
			virtual ~SerialPort();

			int get(char* buf, unsigned char numBytes) throw (SerialException);
			void send(const char* buf, size_t numBytes) throw (SerialException);
			void flushInput();
			void setSpeed(const tcflag_t& baud);
			void setTimeout(unsigned int deciseconds = 0);
			/** Gets path associated with device.
			 *  @return path associated with device
			 */
			const std::string& getPath() { return path; }
		private:
			bool block;
			termios options;
			std::string path;
	};
}

#endif /*SERIALPORT_H_*/
