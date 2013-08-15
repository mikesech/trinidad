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

#ifndef SERLCD_H_
#define SERLCD_H_

#include "../SerialPort.h"
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <ctime>

namespace urt {
namespace contrib {

/** Enables control of a Sparkfun SerLCD v2.5 board through a serial port.
  * This class enables the use of a 16 character by 2 row serial board with
  * a Sparkfun SerLCD v2.5 control board. It allows directly writing characters to
  * the screen as well as iterating through key, value pairs displayed on the bottom
  * line in a "key: value" fashion.
  *
  * @note No line control is provided. If you attempt to write too many characters to one line,
  * the content will wrap to the next.
  *
  * There are two ways to use the board. Write strings (or objects convertible to strings, such as ints
  * or doubles) via the operator<<(). Special const strings are provided -- clear, top, bottom, and clear_bottom --
  * which clear the screen, set the cursor to the top, set the cursor to the bottom, and clear the bottom respectively.
  * They should be used with the operator<<().
  *
  * @note This class is not an ostream. It will not accept standard ostream manipulators like endl and setw.
  *
  * Another way to use the screen is through the strobe method. Using setStrobeLine(), you can set key/value pairs.
  * When strobe() is called and it has been at least STROBE_INTERVAL_SEC seconds (presently hardcoded at 1), the bottom
  * line will iterate to the next key/value pair. This method is intended to be used with the URT State system, connecting
  * setStrobeLine() to a substate's signal and strobe() to an EventLoop's interval signal.
  */
class SerLCD : public urt::SerialPort {
public:
	/** Create a SerLCD object.
	  * @param dev Path to serial port device file.
	  */
	SerLCD(const char* dev);
	
	/** Iterate to the next key/value pair to display on bottom line. */
	void strobe();
	/** Set key/value pair to strobe through on bottom line.
	  * @param title The title or key of this pair.
	  * @param value The value of this pair.
	  */
	void setStrobeLine(const std::string& title, const std::string& value);
	/** Remove key/value pair of given title from strobe process.
	  * @param title Title or key of pair to remove.
	  */
	void delStrobeLine(const std::string& title);
	
	/** Print value at current cursor point.
	  * @tparam T Type of value to print (must be convertible by boost::lexical_cast to std::string).
	  * @param s Value to print. */
	template<typename T>
	SerLCD& operator<<(T& s) {
		return (*this)<<boost::lexical_cast<std::string>(s);
	}
	/** Print string at current cursor point.
	  * @param s String to print. */
	SerLCD& operator<<(const std::string& s);
	
	/** Clear the screen. Use with operator<<(). */
	static const char* clear;
	/** Move cursor to the top. Use with operator<<(). */
	static const char* top;
	/** Move cursor to the bottom. Use with operator<<(). */
	static const char* bottom;
	/** Clear and move cursor to the bottom. Use with operator<<(). */
	static const char* clear_bottom;
	
private:
	bool onActivity() { return true; }

	static const int STROBE_INTERVAL_SEC = 1;
	size_t heartbeat;
	
	typedef boost::unordered_map<std::string, std::string> LineMap;
	LineMap lines;
	LineMap::const_iterator lineItr;
	std::time_t lastStrobe;
};

}
}

#endif
