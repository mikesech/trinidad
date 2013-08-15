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

#include "Ax3500.h"
#include <cctype>

#include "../Log.h"
#include "../State.h"
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <cctype>
using boost::lexical_cast;

using namespace urt::contrib;

Ax3500::Ax3500(const char* dev) throw (urt::SerialException)
	: urt::SerialPort(dev, B9600, false, true, CS7, urt::EVEN) {
	reset();
}
Ax3500::~Ax3500() {
	//reset(); //Reset so that the controller will stop do anything, for safety!
	try {
		send("%rrrrrr\r", 8); //send reset command
	} catch(...) {} //There's a chance we're being deleted because the port is bad.
	//We don't actually call reset(), since reset() also gets the status as well,
	//and we really don't care since we're destroying the connection.
}


void Ax3500::reset() {
	send("%rrrrrr\r", 8); //send reset command
	sleep(1);
	send("^00\r", 4); //Get the watchdog state along with the standard reset info
	expectedResponses.push(RESET);
}

void Ax3500::resetWatchdog() {
	send("", 1); //send null character
}

//Only for two digit hex values (0-255)
static inline char* toHex(unsigned char val, char* out) {
	out[1] = val % 16;
	if(out[1] > 9)
		out[1] += 'a' - 10;
	else
		out[1] += '0';
		
	out[0] = val / 16;
	if(out[0] > 9)
		out[0] += 'a' - 10;
	else
		out[0] += '0';
	return out;
}
//Only for two digit hex values (00-FF) WITH leading 0 if necessary
static inline unsigned char fromHex(const char* in) {
	char val;
	if(std::isdigit(in[0]))
		val = (in[0] - '0') * 0x10;
	else
		val = (std::toupper(in[0]) - 'A' + 10) * 0x10;
	
	if(std::isdigit(in[1]))
		val += in[1] - '0';
	else
		val += std::toupper(in[1]) - 'A' + 10;
	return val;
}

void Ax3500::setSpeed(Channel channel, Direction direction, char speed) throw(std::logic_error) {
	if(speed > MAX_SPEED || speed < 0)
		throw std::logic_error("invalid speed given");

	char cmd[5];
	cmd[0] = '!';
	switch(channel) {
		case LINEAR:   cmd[1] = 'a'; break;
		case STEERING: cmd[1] = 'b'; break;
	}
	if(direction == FORWARD)
		cmd[1] -= 'a' - 'A';
	toHex(speed, &cmd[2]);
	cmd[4] = '\r';
	send(cmd, 5);
	
	expectedResponses.push(COMMAND);
}

void Ax3500::setLinearSpeed(const std::string& key, const std::string& value) {
try {
	const int speed = lexical_cast<int>(value);
	if(speed < 0)
		setSpeed(LINEAR, REVERSE, -speed);
	else
		setSpeed(LINEAR, FORWARD,  speed);
} catch(...) {} //ignore errors
}
void Ax3500::setSteeringSpeed(const std::string& key, const std::string& value) {
try {
	int speed = lexical_cast<int>(value);
	if(speed < 0)
		setSpeed(STEERING, COUNTER,  -speed);
	else
		setSpeed(STEERING, CLOCKWISE, speed);
} catch(...) {} //ignore errors
}

void Ax3500::setPIDGain(PIDChannel channel, char value) {
try {
	if(channel < P1 || channel > D2)
		throw std::invalid_argument("channel given is invalid");
	if(value < 0 || value > MAX_PID_GAIN)
		throw std::invalid_argument("value given is out of bounds");
  
	char cmd[7];
	cmd[0] = '^';
	toHex(channel, &cmd[1]);
	cmd[3] = ' ';
	toHex(value, &cmd[4]);
	cmd[6] = '\r';
	send(cmd, 7);
	expectedResponses.push(COMMAND);
	
} catch(...) {} //ignore errors
}

bool Ax3500::onActivity() {
	std::string msg;
	char c;
	get(&c, 1);
	
	//we weren't expecting anything, see if the watchdog timedout
	if(expectedResponses.empty()) {
		if(c == 'W') //if it did, reset it
			resetWatchdog();
		else //Uh-oh. Kill device.
			return false;
	} else {
		while (c != '\r') {
			msg += c;
			get(&c, 1);
		}
		msg += '\n';
	
		switch(expectedResponses.front()) {
			case COMMAND: {
				get(&c, 1); //get success flag (+ on success, - on failure)
				get(&c, 1); //skip carriage return
				//ignore failures
			} break;
			case WATCHDOG_RESET: break;
			case RESET: {
				//forget the command echo
				msg.clear();
				//get next four information lines
				for(int i = 0; i < 5; i ++) {
					get(&c, 1);
					while (c != '\r') {
						msg += c;
						get(&c, 1);
					}
					msg += '\n';
				}
				//forget watchdog request echo
				get(&c, 1);
				while (c != '\r') get(&c, 1);
				//get watchdog info
				get(&c, 1); //0
				get(&c, 1); //info byte
				if(c == '1')
					urt::Log::msg<<"Ax3500 reset info:\n"<<msg<<"Watchdog disabled.\n"<<std::endl;
				else if(c == '2')
					urt::Log::msg<<"Ax3500 reset info:\n"<<msg<<"Watchdog enabled.\n"<<std::endl;
				else
					urt::Log::msg<<"Ax3500 reset info:\n"<<msg<<"Not in RS232 mode.\n"<<std::endl;
				//finish up line
				get(&c, 1);
				while (c != '\r') get(&c, 1);
				//get success line (yeah, we just did a read, but it returns success/failure anyways)
				get(&c, 1);
				while (c != '\r') get(&c, 1);
			} break;
			case MAIN_BATT: {
				//forget the command echo
				msg.clear();
				//get next two information lines
				for(int i = 0; i < 2; i ++) {
					get(&c, 1);
					while (c != '\r') {
						msg += c;
						get(&c, 1);
					}
					//We only care about the first line, but we do need to dump the other.
					if(i == 0)
						urt::State::set(m_mainBattKey, 55*fromHex(msg.c_str())/256.0);
					msg += '\n';
				}
			} break;
				
		}
		expectedResponses.pop();
	}
	return true;
}

//Queries
void Ax3500::requestBatteryVoltage() {
	if(m_mainBattKey.empty()) return;
	send("?e\r", 3);
	expectedResponses.push(MAIN_BATT);
}
