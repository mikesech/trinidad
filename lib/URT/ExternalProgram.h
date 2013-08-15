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

/*
 * ExternalProgram.h
 *
 *  Created on: Mar 16, 2010
 *      Author: Michael Sechooler
 */

#ifndef EXTERNALPROGRAM_H_
#define EXTERNALPROGRAM_H_

#include "urtexcept.h"
#include <vector>
#include <cstdarg>

namespace urt {

namespace internal {
	/** Internal base class holding all common code for template class ExternalProgram; should not be used elsewhere. */
	class _ExternalProgram {
	protected:
		static const int SIGDELAY = 100000; ///< Delay in microseconds used between TERM and KILL signals sent to children processes
		_ExternalProgram(const char *const argv[], bool connectStderr) throw (ChildProcessException);
		~_ExternalProgram();

		int sockets[2]; ///< Sockets associated with external program.
		int childPid; ///< PID of child process.
	};
}

/**
 * This class permits the use and control of external child processes.
 *
 * Unfortunately, URT is not thread-safe at all. The use of the monostate/static State class, static
 * buffers in the StateSocket and ArdPort classes, and signals prohibits the use of threads with all
 * URT classes. However, the URT library does provide a more robust and useful alternative: ExternalProgram.
 *
 * The ExternalProgram class executes an external program as a child process, connecting its stdin, stdout, and
 * stderr to a socket. In this manner, information can be communicated between the main process and its children.
 *
 * @attention The socket is parameterized as SocketType. This class works by deriving from it. It is extremely important
 * 	that SocketType's destructor is virtual; if it is not, memory leaks and zombie processes will result.
 *
 * @note Currently, there is no way to distinguish between stdout and stderr if both are connected to the ExternalProgram
 * 	object.
 *
 * @note The socket is a UNIX local socket, not a network TCP socket, so reliability and low latency
 * is assured.
 *
 * @bug If the main process is terminated or killed by a signal, any children processes will not be terminated as normal.
 * 	Instead, they will be left as orphan processes.
 *
 * @tparam SocketType the type of socket to use to interface with the child process; must have a constructor that takes
 *  	only the file descriptor of the new socket
 */
template <typename SocketType>
class ExternalProgram : private internal::_ExternalProgram, public SocketType {
public:
	/**
	 * Run an external process.
	 * @param argv an array consisting of the program to execute and arguments terminated with a null pointer
	 * @param connectStderr if true, stderr is connected to the socket; if false, stderr remains the same as the main process'
	 * @note The program name can be either a path or a name. The system will use the shell to find the proper executable.
	 *
	 * @warning If the program is unable to run, due to a bad filename or whatever, the constructor will not necessarily fail.
	 * 	More likely, an error event will be generated for the onActivity() of SocketType to handle.
	 * @throws ChildProcessException thrown if unable to create the UNIX socket pair or fork a new process
	 */
	ExternalProgram(const char *const argv[], bool connectStderr = false) throw (ChildProcessException)
		: internal::_ExternalProgram(argv, connectStderr), SocketType(sockets[0]) {}
#ifdef __GXX_EXPERIMENTAL_CXX0X__
	/**
	 * This version of Create() uses the new varadic template feature of the upcoming c++0x standard to provide a type-safe varadic function.
	 * Currently, it's used in place of the cstdargs type of varadic function only on g++ compliers invoked with the -std=gnu++0x or -std=c++0x.
	 * @note You can end the argument list with or without a null pointer. However, it is suggested that you do to enable seamless compatibility
	 * with the non-C++0x version of this function. Moreover, not doing so will create a problem, since the code will compile properly without
	 * gnu c++0x support, even though it will exhibit undefined behavior without a terminal null pointer.
	 */
	template<typename ... Args>
	static ExternalProgram<SocketType>* Create(bool connectStderr, const char* file, Args&& ... args) throw (ChildProcessException) {
		const char *const a[] = {file,args..., (const char*)0};
		return new ExternalProgram<SocketType>(a, connectStderr);
	}
#else
	/**
	 * Create an ExternalProgram object, running an external process.
	 * This function is similar to ExternalProgram(), except it takes a variable number of parameters as the process arguments
	 * as opposed to an array.
	 * @attention Make sure that the last parameter to this function is a NULL pointer. Doing otherwise will result in terribly
	 * 	horrible behavior and will result in a compile-time warning. Note that a warning will still be generated if passing
	 *  the integer 0; generally, the 0 needs to be cast to a <tt>const char*</tt> first. (In fact, this is absolutely necessary
	 *  on systems where an \c int is differently sized from a pointer. Not doing so will result in horribly undefined behavior.)
	 * @param connectStderr if true, stderr is connected to the socket; if false, stderr remains the same as the main process'
	 * @param file name or path of program to run
	 * @param ... remainder of process arguments in const char* form, terminated with a NULL pointer
	 */
	static ExternalProgram<SocketType>* Create(bool connectStderr, const char* file, ...) throw (ChildProcessException) __attribute__ ((sentinel)) {
		va_list list;
		va_start(list, file);
		std::vector<const char*> args;
		args.push_back(file);
		do {
			args.push_back(va_arg(list, const char*));
		} while(args.back() != NULL);
		va_end(list);
		return new ExternalProgram<SocketType>(&args[0], connectStderr);
	}
#endif

	/**
	 * Terminate an external process.
	 * If the process has not already terminated, a TERM signal will be sent followed by a delay of internal::_ExternalProgram::SIGDELAY microseconds.
	 * If it has not terminated by then, a KILL signal will be sent followed again by an identical delay.
	 * If it has not terminated by then, it will be dealt with later when the main process terminates and it becomes an orphan process.
	 */
	~ExternalProgram() {}
};

}

#endif /* EXTERNALPROGRAM_H_ */
