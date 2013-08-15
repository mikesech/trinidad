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

#ifndef EXCEPTURT_H_
#define EXCEPTURT_H_

#include <stdexcept>

/** @file urtexcept.h
 * This file holds the definitions of all URT-specific exceptions. All exceptions
 * are descendants of std::exception; as such, they support standard exception features,
 * such as the what() function call.
 */

/** Create a new exception class by deriving from a parent exception. Used instead of a simple typedef to provide
  * RTTI, especially the ability to distinguish between different exceptions in a try/catch block. */
#define URT_DEFINE_EXCEPTION(name, type) class name : public type { public: name(const std::string& msg) : type(msg) {} }

namespace urt
{
	/**@defgroup exceptions URT Exceptions
	* All the special exceptions used by URT. All exceptions
	* are descendants of std::exception; as such, they support standard exception features,
	* such as the what() function call.
	* @{
	*/

	/** Generated when unable to open, read to, or write to serial port. */
	URT_DEFINE_EXCEPTION(SerialException, std::runtime_error);

	/** Generated when a socket error occurs. */
	URT_DEFINE_EXCEPTION(SocketException, std::runtime_error);

	/** Generated when an error occurs with respect to a child process. */
	URT_DEFINE_EXCEPTION(ChildProcessException, std::runtime_error);
	
	/** Generated when an error occurs while creating or operating a timer.
	  * This exception is extremely rare and indicates a significant underlying issue. */
	URT_DEFINE_EXCEPTION(TimerException, std::runtime_error);

	/*@}*/
}

#endif /* EXCEPTURT_H_ */
