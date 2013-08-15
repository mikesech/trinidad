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

#ifndef HOTDEVICEMANAGER_H_
#define HOTDEVICEMANAGER_H_

#include "DeviceManager.h"
#include "FDEvtSource.h"

#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#ifdef __linux__
#include <linux/version.h>
#endif
#if !defined(__linux__) || LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
#error HotDeviceManager uses inotify, requiring a linux kernel version equal to or greater than 2.6.13.
#endif

namespace urt {

/**
  * Functions as a hot-pluggable DeviceManager. Whereas the normal DeviceManager 
  * examines directories only when execute() is called, a HotDeviceManager
  * monitors changes in the directories during execution, enabling the recognition
  * and use of devices plugged in during execution.
  *
  * HotDeviceManager works by using the inotify feature introduced to the Linux kernel since version
  * 2.6.13. As such, it will only work with Linux and only with those kernels at or above that version
  *
  * @note Unlike StateDeviceNotifier, the HotDeviceManager does not watch for the deletion of device
  * files. In theory, should a device no longer exist, the file descriptor will enter an error state
  * and generate an event; watching for the deletion of device files, therefore, is unnecessary.
  */
class HotDeviceManager : public DeviceManager, public urt::FDEvtSource {
public:
	/**
	 * Create a HotDeviceManager. All new StateDevices found during execution will be added to the given loop.
	 * @param loop EventLoop to which to add new StateDevices found during execution.
	 * @note The HotDeviceManager must be added to an EventLoop to watch for changes during execution.
	 * This is not done automatically. Furthermore, this means that the HotDeviceManager must be created on the heap
	 * (with the new keyword) like any other FDEvtSource used with an EventLoop.
	 */
	HotDeviceManager(EventLoop& loop);
	//Documentation found in DeviceManager.h
	bool addDirectory(const std::string& dir, const std::string& rule);
	/**
	 * Delete a HotDeviceManager.
	 */
	~HotDeviceManager();
	
	using DeviceManager::execute;

private:
	EventLoop& m_loop;
	boost::unordered_map<int, Directory*> m_directoryMap;
	
	bool onActivity();
};

}

#endif
