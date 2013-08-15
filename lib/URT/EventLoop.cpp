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

#include "EventLoop.h"
#include "FDEvtSource.h"
#include "Log.h"
#include <typeinfo>
#include <poll.h>
#include <algorithm>
#include <set> //required for std::greater<>() comparator generator

using namespace urt;


EventLoop::EventLoop(int timeout) : timeout(timeout), adjustedTimeout(timeout), running(false) {
	timeLastInterval.tv_sec = 0;
	timeLastInterval.tv_nsec = 0;
}
//EventLoop::~EventLoop() {}

/**
 * Start processing events.
 */
void EventLoop::run()
{
	running = true;
	while(!fds.empty())
	{
		if(poll(&fds[0], fds.size(), adjustedTimeout) > 0)
		{
			for(std::vector<pollfd>::iterator i = fds.begin(); i < fds.end(); i++)
			{
				if(i->revents & (POLLIN | POLLERR | POLLHUP | POLLRDHUP))
					if(!fdsources[i - fds.begin()]->onActivity())
						deleteQueue.push_back(i - fds.begin());
			}
		}

		//safe to remove from fds. take care of queue now
		std::sort(deleteQueue.begin(), deleteQueue.end(), std::greater<size_t>());
		for(std::vector<size_t>::iterator i = deleteQueue.begin(); i < deleteQueue.end(); i++)
		{
			//Note, this relies on the fact that the deleteQueue will be sorted (so we can remove in descending order,
			//avoiding changing the following indexes).
			Log::msg<<typeid(**(fdsources.begin() + *i)).name()<<" deleted"<<std::endl;
			fds.erase(fds.begin() + *i);
			fdsources.erase(fdsources.begin() + *i);
		}
		deleteQueue.clear();

		//safe to add sources
		while(!addQueue.empty())
		{
			fdsources.push_back(addQueue.front());
			pollfd t = {addQueue.front()->fdesc, POLLIN | POLLRDHUP, 0};
			fds.push_back(t);
			addQueue.pop();
		}

		//check to see if time to call interval handler; adjust adjustedTimeout
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		long long diff = (now.tv_sec - timeLastInterval.tv_sec)*(long long)1000 + (now.tv_nsec - timeLastInterval.tv_nsec)/1000000;
		if(diff >= timeout)
		{
			intervalSignal();
			adjustedTimeout = timeout;
			clock_gettime(CLOCK_MONOTONIC, &timeLastInterval);
		}
		else
		{
			adjustedTimeout = timeout - diff;
			if(adjustedTimeout < 0)
				adjustedTimeout = 0;

		}
	}
	running = false;
}

/**
 * Get the internal index given to an FDEvtSource. Should only be called internally.
 * @note Only compares address of FDEvtSource, not actual attributes.
 * @param fdsource pointer to FDEvtSource to check
 * @return index of FDEvtSource or number of FDEvtSources registered if not found
 */
size_t EventLoop::indexFDSource(FDEvtSource* fdsource)
{
	for(std::vector<boost::shared_ptr<FDEvtSource> >::iterator i = fdsources.begin(); i < fdsources.end(); i++)
		if(i->get() == fdsource)
			return i - fdsources.begin();
	return fdsources.size();
}
/**
 * Add an FDEvtSource to event loop.
 * The EventLoop does not take ownership of the FDEvtSource; rather, it shares it with all other shared_ptr owners. Consequently
 * the object will not automatically be deleted when appropriate unless no other shared_ptrs exist.
 * @param fdsource pointer to FDEvtSource to add
 * @return false if particular FDEvtSource (not necessarily file descriptor) already added
 */
bool EventLoop::add(const boost::shared_ptr<FDEvtSource>& fdsource)
{
	if(indexFDSource(fdsource.get()) == fdsources.size())
	{
		if(running)
		{
			addQueue.push(fdsource);
		}
		else
		{
			fdsources.push_back(fdsource);
			pollfd t = {fdsource->fdesc, POLLIN | POLLRDHUP, 0};
			fds.push_back(t);
		}
		return true;
	}
	return false;
}

/**
 * Remove an FDEvtSource from event loop.
 * @note Calling this function does not delete the \c fdsource. Additionally, the \c fdsource is also released from
 * 		EventLoop ownership; it is now the responsibility of some outside force to delete the FDEvtSource.
 * @param fdsource pointer to FDEvtSource to remove
 * @return false if particular FDEvtSource (not necessarily file descriptor) not found
 */
bool EventLoop::remove(FDEvtSource* fdsource)
{
	size_t i = indexFDSource(fdsource);
	if(i < fdsources.size())
	{
		if(running) //can't remove while running, add to queue to delete later
		{
			deleteQueue.push_back(i);
		}
		else
		{
			Log::msg<<typeid(**(fdsources.begin() + i)).name()<<" deleted"<<std::endl;
			fds.erase(fds.begin() + i);
			fdsources.erase(fdsources.begin() + i);
		}
		return true;
	}
	return false;
}

/**
 * Determines if event loop is watching given FDEvtSource.
 * @param fdsource pointer
 * @return true if watching FDEvtSource
 */
bool EventLoop::has(FDEvtSource* fdsource)
{
	return indexFDSource(fdsource) < fdsources.size();
}

void EventLoop::registerIntervalSlot(const boost::signal<void ()>::slot_type& slot) {
	intervalSignal.connect(slot);
}
