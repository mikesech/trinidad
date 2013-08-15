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

#include "DeviceManager.h"
#include "EventLoop.h"
#include "StateDevice.h"
#include "Log.h"

namespace urt {

DeviceManager::Directory::Directory(const std::string& path, const std::string& rule)
	: m_directory(path), m_rule(rule, boost::regex::extended) {
	if(!boost::filesystem::is_directory(path))
		throw Error("not a directory");
}

bool DeviceManager::addDirectory(const std::string& dir, const std::string& rule) {
	return addExposedDirectory(dir, rule);
}
DeviceManager::Directory* DeviceManager::addExposedDirectory(const std::string& dir, const std::string& rule) {
	try {
		m_directories.push_back(Directory(dir, rule));
	} catch(...) {
		return NULL;
	}
	return &m_directories.back();
}


void DeviceManager::execute(EventLoop& loop) {
	static const boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
	for(std::list<Directory>::iterator d = m_directories.begin(); d != m_directories.end(); d++) {
		for(boost::filesystem::directory_iterator i(d->m_directory); i != end_itr; ++i) {
			if(boost::regex_match(i->leaf(), d->m_rule)) {
				urt::Log::msg<<"Found "<<*i<<std::endl;
				try {
					boost::shared_ptr<StateDevice> device(new urt::StateDevice(i->string().c_str()));
					
					if(onFound(device)) {
						loop.registerIntervalSlot(&urt::StateDevice::poll, *device);
						loop.add(device);
					} else {
						urt::Log::err<<"Error loading "<<*i<<". Vetoed."<<std::endl;
					}
				} catch (std::exception& e) {
					m_failed.push_back(i->string());
					urt::Log::err<<"Error loading "<<*i<<". "<<e.what()<<std::endl;
				}
			}
		}
	}
}

}
