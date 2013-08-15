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

#include "HotDeviceManager.h"
#include "StateDevice.h"
#include "Log.h"
#include "EventLoop.h"
#include <sys/inotify.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <unistd.h>

namespace urt{

HotDeviceManager::HotDeviceManager(EventLoop& loop) : m_loop(loop) {
	fdesc = inotify_init1(IN_CLOEXEC);
}

HotDeviceManager::~HotDeviceManager() {
	close(fdesc);
}

bool HotDeviceManager::addDirectory(const std::string& dir, const std::string& rule) {
	Directory* d = addExposedDirectory(dir, rule);
	if(d == NULL)
		return false;

	m_directoryMap[inotify_add_watch(fdesc, dir.c_str(), IN_CREATE)] = d;
	return true;
}

bool HotDeviceManager::onActivity() {
	char buf[sizeof(inotify_event)+1024];
	inotify_event* event = reinterpret_cast<inotify_event*>(buf);
	ssize_t r = read(fdesc, buf, sizeof(buf));
	if(r <= 0)
		throw std::logic_error("inotify event with impossibly large name received");
	
	while(r > 0) {
		Directory* d = m_directoryMap[event->wd];
		
		if(boost::regex_match(event->name, d->m_rule)) {
			boost::filesystem::path absPath = d->m_directory; absPath /= std::string(event->name);
			Log::msg<<"Detected newly added "<<absPath<<std::endl;
			StateDevice* device = NULL;
			try {
				device = new StateDevice(absPath.string().c_str());
			} catch(...) {
				try {
					usleep(500000); //sleep for .5 seconds so device file can stabilize and try again
					device = new StateDevice(absPath.string().c_str());
				} catch (std::exception& e) {
					Log::err<<"Error loading "<<absPath<<". "<<e.what()<<std::endl;
					delete device; //if construction failed, it will be null and okay to delete
					
					addFailed(absPath.string());
				}
			}
			if(device != NULL) {
				boost::shared_ptr<StateDevice> ptr(device);
				if(onFound(ptr)) {
					m_loop.registerIntervalSlot(&urt::StateDevice::poll, *ptr);
					m_loop.add(ptr);
				} else {
					urt::Log::err<<"Error loading "<<absPath<<". Vetoed."<<std::endl;
				}
			}
		}
			
		r -= sizeof(inotify_event) + event->len;
		event = reinterpret_cast<inotify_event*>(reinterpret_cast<char*>(event) + sizeof(inotify_event) + event->len);
	}
	return true;
}

}
