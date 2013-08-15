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
 * Log.h
 *
 *  Created on: Mar 16, 2010
 *      Author: Michael Sechooler
 */

#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <iostream>

namespace urt {

/**
 * The Log static class provides a uniform way to output warnings and errors. It should be used
 * as opposed to directly writing to stdout and stderr to enable flexibility in the future when
 * the ability to seamlessly direct messages to any file is added.
 */
class Log {
public:
	/**
	 * Outputs a message-level message to the associated log file (defaults to stderr).
	 * @tparam T iostream-supported message type
	 * @param msg message to output
	 */
	template<typename T>
	static void message(const T& msg) { Log::msg<<msg<<std::endl; }

	/**
	 * Outputs a warning-level message to the associated log file (defaults to stderr).
	 * @tparam T iostream-supported message type
	 * @param msg message to output
	 */
	template<typename T>
	static void warning(const T& msg) { warn<<"Warning: "<<msg<<std::endl; }

	/**
	 * Outputs a error-level message to the associated log file (defaults to stderr).
	 * @tparam T iostream-supported message type
	 * @param msg message to output
	 */
	template<typename T>
	static void error(const T& msg) { err<<"ERROR: "<<msg<<std::endl; }

	static std::ostream& msg; ///< Alias for ostream to use for messages.
	static std::ostream& warn; ///< Alias for ostream to use for warnings.
	static std::ostream& err; ///< Alias for ostream to use for errors.
private:
	Log() {}
};

}

#endif /* LOG_H_ */
