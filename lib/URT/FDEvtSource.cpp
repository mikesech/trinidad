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

#include "FDEvtSource.h"
#include "EventLoop.h"

using namespace urt;

/**
 * Creates an event handler with the protected variable fdesc set later.
 * @warning Calling any other function before setting fdesc will result in undefined behavior.
 */
FDEvtSource::FDEvtSource() : fdesc(-1) {}

/**
 * Creates an event handler for a file descriptor.
 * @param fd file descriptor to watch
 */
FDEvtSource::FDEvtSource(int fd) : fdesc(fd) {}

FDEvtSource::~FDEvtSource() {}


/**@fn FDEvtSource::onActivity
 * Pure virtual function called when something happens to the file descriptor (either there is data to be read,
 * or an error has occurred).
 * @return return false only to indicate that the event loop should deregister the source (i.e, on an error); otherwise, return true
 */
/**@var FDEvtSource::fdesc
 * File descriptor to watch. Can either be set by subclass or at instantiation.
 * @attention Must be set prior to using any function. Do not change while attached to event loop.
 */
