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

#ifndef FDEVTSOURCE_H_
#define FDEVTSOURCE_H_
#include <boost/utility.hpp>
#include <boost/signals/trackable.hpp>

namespace urt
{
	class EventLoop;
	/** This abstract base class forms the basis of all event-driven (asynchronous) file I/O.
	 *
	 *  The file associated with this class does not need to be regular; it may be a device file such as a serial
	 *  port. The only requirement is that the file has a normal file descriptor that works with poll().
	 *
	 *  Generally, implementing some form of event-driven file I/O involves the following:
	 *  <ol>
	 *  <li>Extend this class</li>
	 *  <li>Ensure that the subclass stores the associated file descriptor in \c fdsec or passes it along in the constructor.</li>
	 *  <li>Implement onActivity() using normal system calls to read from the file. If data is still available after
	 *			the handler is complete, another event will be generated.</li>
	 *  </li>
	 *  </ol>
	 *
	 *  @note No events are generated while writing to a file descriptor; all writing I/O is synchronous.
	 */
	class FDEvtSource : public boost::signals::trackable, boost::noncopyable
	{
		public:
			FDEvtSource();
			FDEvtSource(int fd);
			virtual ~FDEvtSource();

		protected:
			virtual bool onActivity() = 0;
			int fdesc;

		private:
			//FDEvtSource(const FDEvtSource&); //not copyable
			//FDEvtSource& operator=(const FDEvtSource&); //not copyable
			friend class EventLoop;
	};
}

#endif /* FDEVTSOURCE_H_ */
