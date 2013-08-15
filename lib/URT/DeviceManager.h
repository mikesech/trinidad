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

#ifndef DEVICEMANAGER_H_
#define DEVICEMANAGER_H_

#include <string>
#include <list>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>

namespace urt {

class EventLoop;
class StateDevice;

/**
 * Finds actual StateDevices, creates object for them, and adds
 * them to EventLoop according to given rules.
 *
 * The DeviceManager searches given directories for device files that
 * match a given regular expression rule. Upon a match, a new StateDevice
 * object is created and added to a given EventLoop; additionally, its poll() is
 * connected to the EventLoop's interval signal.
 *
 * For further control of the StateDevice, extend this class and override the virtual
 * onFound(). Returning false from that function will veto the StateDevice and destroy it,
 * provided you do not keep or copy the boost::shared_ptr passed as a parameter. One example
 * use of this function is to attach motor-related substates to the StateDevice's sendSubstate().
 */
class DeviceManager {
public:
	/**
	  * Add a new directory to be searched with regex rule against which to match filenames.
	  * Does not immediately search for devices; call execute() after adding all directories
	  * to execute search.
	  * @param dir Directory to search.
	  * @param rule Extended POSIX regular expression against which to match filenames.
	  * @return True if successful. False if directory does not exist or rule is malformed.
	  * @note It is unadvised to add a directory more than once.
	  */
	virtual bool addDirectory(const std::string& dir, const std::string& rule);
	/**
	  * Execute search using given rules.
	  * @param loop EventLoop to which to add found devices.
	  * @note Do not call more than once!
	  */
	void execute(EventLoop& loop);
	
	/**
	  * Virtual function called upon finding a matching device file and after creating
	  * a StateDevice object for it. Does nothing by default.
	  * @param device The newly created StateDevice.
	  * @return False to veto object (destroying it without adding to event loop). True otherwise.
	  */
	virtual bool onFound(boost::shared_ptr<StateDevice> device) { return true; }
	
	/**
	  * Gets a list of the paths for all devices that failed to handshake.
	  * @note If onFound() vetoed the connection, the device will not be added
	  * to the list since the handshake had to be successful.
	  * @return List of all the paths for all devices that failed to handshake.
	  */
	const std::list<std::string>& getFailed() { return m_failed; }
	
	/**
	  * Resets list of failed devices.
	  */
	void resetFailed() { m_failed.clear(); }

protected:
	struct Directory {
		Directory(const std::string& path, const std::string& rule);

		const boost::filesystem::path m_directory;
		const boost::regex m_rule;
		
		typedef std::runtime_error Error;
	};
	//Like addDirectory(), but returns pointer to directory structure for use by child classes.
	//Returns NULL on failure.
	Directory* addExposedDirectory(const std::string& dir, const std::string& rule);
	
	void addFailed(const std::string& s) { m_failed.push_back(s); }

private:
	std::list<Directory> m_directories;
	std::list<std::string> m_failed;
};

}

#endif
