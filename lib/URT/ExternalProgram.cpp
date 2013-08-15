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
 * ExternalProgram.cpp
 *
 *  Created on: Mar 16, 2010
 *      Author: Michael Sechooler
 */

#include "ExternalProgram.h"

#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

namespace urt {

internal::_ExternalProgram::_ExternalProgram(const char *const argv[], bool connectStderr) throw (ChildProcessException) {
	if(socketpair(AF_LOCAL, SOCK_STREAM, 0, sockets) == -1)
		throw ChildProcessException("Error opening UNIX domain socket pair");

	int childPid = vfork();
	if(!childPid) { //child
		close(sockets[0]); //close other socket
		dup2(sockets[1], 0); //connect stdin to socket
		dup2(sockets[1], 1); //connect stdout to socket
		if(connectStderr)
			dup2(sockets[1], 2); //connect stderr to socket
		execvp(argv[0], const_cast<char *const*>(argv));
		_exit(1); //if we're here, the execvp() call failed. this process needs to die, now.
	} else if(childPid == -1) { //error
		close(sockets[0]);
		close(sockets[1]);
		throw ChildProcessException("Error forking");
	}
	close(sockets[1]); //close other socket
	_ExternalProgram::childPid = childPid; //save child pid
}

internal::_ExternalProgram::~_ExternalProgram() {
	if(waitpid(childPid, NULL, WNOHANG) <= 0) { //still running
		kill(childPid, SIGTERM); //send child process the term signal
		usleep(SIGDELAY); //wait 1/10th second
		if(waitpid(childPid, NULL, WNOHANG) <= 0) { //still running
			kill(childPid, SIGKILL); //send kill signal
			usleep(SIGDELAY);
			waitpid(childPid, NULL, WNOHANG); //try to reap child process one last time
		}
	}
	close(sockets[0]);
}

}
