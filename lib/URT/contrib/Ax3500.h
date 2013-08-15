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

#ifndef _AX3500_H_
#define _AX3500_H_

#include "../SerialPort.h"

#include <string>
#include <queue>
#include <stdexcept>

namespace urt {
namespace contrib {

/**
  * The Ax3500 class represents a physical Roboteq AX3500 motor controller board.
  * It permits client code to access the board over a serial port and issue a
  * variety of commands.
  */
class Ax3500 : public urt::SerialPort {
public:
	enum Channel {
		LINEAR, RIGHT = LINEAR, STEERING, LEFT = STEERING
	};
	enum Direction {
		FORWARD, CLOCKWISE = FORWARD, REVERSE, COUNTER = REVERSE
	};
	/** The different PID gain values available on the AX3500. */
	enum PIDChannel {
		P1 = 0x82, 	///< Proportional, channel 1 (right)
		P2, 		///< Proportional, channel 2 (left)
		I1, 		///< Integral, channel 1 (right)
		I2, 		///< Integral, channel 2 (left)
		D1, 		///< Differential, channel 1 (right)
		D2 		///< Differential, channel 2 (left)
	};
	/** Maximum speed value, inclusive. */
	static const int MAX_SPEED = 127;
	/** Maximum PID gain value, inclusive. */
	static const int MAX_PID_GAIN = 63;

	/** Construct an Ax3500 object and connect to it immediately.
	  * @note Watchdog mode is saved to the Ax3500's flash memory and
	  *	therefore will be active if and only if it was active during
	  *	the last time of use.
	  * @throws urt::SerialException thrown on error
	  */
	Ax3500(const char* dev) throw (urt::SerialException);
	/** Shutdown Ax3500 and disconnect. */
	~Ax3500();
	
	/**
	 * Reset controller.
	 * Sends the reset message to the Roboteq.
	 */
	void reset();
	
	/** Reset watchdog timer on AX3500. */
	void resetWatchdog();
	
	/**
	  * Sets speed of a channel in a given direction.
	  * @param channel channel to change
	  * @param direction direction to set
	  * @param speed speed to set (constrained from 0 to 127, where 0 is off and 127 is full force)
	  * @throws std::logic_error if speed is not valid
	  */
	void setSpeed(Channel channel, Direction direction, char speed) throw(std::logic_error);

	/**
	  * Sets linear speed using string value. Intended to be used as a substate slot.
	  * @param key ignored
	  * @param value text form of value to set linear/forward channel (negative to indicate reverse direction)
	  */
	void setLinearSpeed(const std::string& key, const std::string& value);
	/** Sets right channel (A) speed. Alias for setLinearSpeed(). */
	inline void setRightSpeed(const std::string& key, const std::string& value) { setLinearSpeed(key, value); }
	/**
	  * Sets steering speed using string value. Intended to be used as a substate slot.
	  * @param key ignored
	  * @param value text form of value to set linear/forward channel (negative to indicate counter-clockwise direction)
	  */
	void setSteeringSpeed(const std::string& key, const std::string& value);
	/** Sets left channel (B) speed. Alias for setSteeringSpeed(). */
	inline void setLeftSpeed(const std::string& key, const std::string& value) { setSteeringSpeed(key, value); }
	
	/**
	  * Temporarily sets the gain for a specific PID channel/type. System uses default setting stored in flash upon reset. 
	  * @param channel the type (P, I, or D) and channel to set (channel 1 refers to the right channel; channel 2, to the left)
	  * @param value the value to set from 0-63; the board internally divides by 8 (so each increment equals .125)
	  * @throws std::invalid_argument if channel or value is invalid
	  */
	void setPIDGain(PIDChannel channel, char value);
	
	
	/* Queries
	 * Queries are conducted asynchronously and are put into the registered substate.
	 */
	/** Requests main battery voltage.
	  * @note Does nothing if key not registered.
	  */
	void requestBatteryVoltage();
	/** Register the key in which to put the battery voltage. 
	  * @param key key in which to put the battery voltage
	  */
	void registerBatteryVoltage(const std::string& key) { m_mainBattKey = key; }	 
	
private:
	enum ExpectedReponse {
		COMMAND, ///< Simple echo of command and +,- indicated success or failure
		WATCHDOG_RESET, ///< Simple echo of command
		RESET, ///< Initialization string
		MAIN_BATT ///< Voltage of main battery in special [0,255] form
	};

	/**
	 * Process incoming messages. Called internally.
	 * @return true if connection still valid
	 */
	bool onActivity();	
	
	std::queue<ExpectedReponse> expectedResponses;
	
	//Registered query keys
	std::string m_mainBattKey;
};

}
}

#endif
