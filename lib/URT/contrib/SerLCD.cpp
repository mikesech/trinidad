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

#include "SerLCD.h"
#include <cstring>

using std::stringstream;
using std::list;

using namespace urt::contrib;

const char* SerLCD::clear = "\376\001";
const char* SerLCD::top = "\376\200";
const char* SerLCD::bottom = "\376\300";
const char* SerLCD::clear_bottom = "\376\300                \376\300";

SerLCD::SerLCD(const char* dev) : urt::SerialPort(dev, B9600), heartbeat(0) {
	std::time(&lastStrobe);
	(*this)<<clear;
}

void SerLCD::strobe() {
	if(!lines.empty()) {
		std::time_t now;
		std::time(&now);
		
		if(now - lastStrobe > STROBE_INTERVAL_SEC) {
			(*this)<<clear_bottom<<lineItr->first<<": "<<lineItr->second;
			if(++lineItr == lines.end())
				lineItr = lines.begin();
				
			lastStrobe = now;
		}
	}
}
void SerLCD::setStrobeLine(const std::string& title, const std::string& value) {
	//check if title/key already set (if so, lineItr won't be invalidated, so don't reset it)
	LineMap::iterator i = lines.find(title);
	if(i == lines.end()) {	
		lines[title] = value;
		lineItr = lines.begin();
	} else
		i->second = value;
}
void SerLCD::delStrobeLine(const std::string& title) {
	lines.erase(title);
	lineItr = lines.begin();
}

SerLCD& SerLCD::operator<<(const std::string& s) {
	send(s.data(), s.size());
	return *this;
}
