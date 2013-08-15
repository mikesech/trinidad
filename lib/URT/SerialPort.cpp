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

#include "SerialPort.h"

#include <cstdio>   //Standard input/output definitions
#include <cstring>  //String function definitions
#include <unistd.h>  //UNIX standard function definitions
#include <fcntl.h>   //File control definitions
#include <errno.h>   //Error number definitions
#include <termios.h> //POSIX terminal control definitions
#include <signal.h>

using namespace urt;

/**
 * Opens serial port.
 * @throw SerialException if unable to open port
 * @param dev path to character file (ex. "/dev/ttyUSB0")
 * @param baud baud rate of type \a tcflag_t (ex. B19200)
 * @param echo enable local echo
 * @param block block on get and send
 * @param dataBits number of frame bit of type \a tcflag_t (ex. CS8)
 * @param parity parity check of type Parity (NONE, ODD, EVEN)
 * @param twoBits use two stops bits as opposed to one
 */
SerialPort::SerialPort(const char* dev, tcflag_t baud, bool echo, bool block, tcflag_t dataBits, Parity parity, bool twoBits) throw (SerialException)
	: block(block), path(dev)
{
	/*
	 * Note: The below differentiation between blocking and non-blocking doesn't appear to actually do anything.
	 * 	 The blocking operation is actually handled in get().
	 */
	if(block)
		fdesc = open(dev, (O_RDWR | O_NOCTTY) & ~O_NONBLOCK); //read-write, block
	else
		fdesc = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK); //read-write, don't block
	if (fdesc == -1)
		throw SerialException("cannot open serial port");

	fcntl(fdesc, F_SETFL, 0);

	tcgetattr(fdesc, &options); //get current (default) options
	cfmakeraw(&options); //set serial port to raw mode (non-canonical, no parity, etc.)

	cfsetispeed(&options, baud); //set baud rate
	cfsetospeed(&options, baud);

	options.c_cflag &= ~CSIZE; //clear data bits field
	options.c_cflag |= dataBits;   //Set data bits

	switch(parity)
	{
		case ODD:
			//Set odd parity
			options.c_cflag |= PARODD;
			options.c_cflag |= PARENB;
			break;
		case EVEN:
			//Set even parity
			options.c_cflag &= ~PARODD;
			options.c_cflag |= PARENB;
			break;
		case NONE: break;
	}

	if(twoBits)
		options.c_cflag |= CSTOPB;
	else
		options.c_cflag &= ~CSTOPB;

	if(echo)
		options.c_cflag |= ECHO;
	options.c_cflag |= (CLOCAL | CREAD); //Enable the receiver and set local mode
	tcsetattr(fdesc, TCSAFLUSH, &options); //Set the new options for the port
	tcflush(fdesc, TCIOFLUSH);
}

/**
 * Closes serial port.
 */
SerialPort::~SerialPort()
{
	close(fdesc);
}

/**
  * Flushes the input buffer.
  */
void SerialPort::flushInput() {
	tcflush(fdesc, TCIFLUSH);
}

/**
  * Changes the baud rate after creation.
  * @note Calling this function will discard the input and output buffer.
  * @param baud Baud rate as done with SerialPort().
  */
void SerialPort::setSpeed(const tcflag_t& baud) {
	cfsetispeed(&options, baud); //set baud rate
	cfsetospeed(&options, baud);
	tcflush(fdesc, TCIOFLUSH);
	tcsetattr(fdesc, TCSANOW, &options);
}
/**
  * Sets the read byte timeout.
  * @param deciseconds Timeout in deciseconds. 0 to disable.
  * @note deciseconds specifies the limit for a timer in tenths of
  *      a second. Once an initial byte of input becomes available, the timer
  *      is restarted after each further byte is received. Because the
  *      timer is only started after the initial byte becomes available, at
  *      least one byte will be read. If the timeout expires, an exception will
  *      be thrown.
  */
void SerialPort::setTimeout(unsigned int deciseconds) {
	options.c_cc[VTIME] = deciseconds;
	tcsetattr(fdesc, TCSANOW, &options);
}

//limited to unsigned char due to maximum VMIN can be set (see .cpp file)
/**
 * Read data from serial buffer.
 * @throw SerialException if unable to read any data
 * @param buf buffer in which to read data
 * @param numBytes number of bytes to read
 * @return Number of bytes read. May not be equal to \a numBytes
 * 		if communication lost for some reason or if using non-blocking I/O.
 */
int SerialPort::get(char* buf, unsigned char numBytes) throw (SerialException)
{
	if(!numBytes) return 0;
	if(block) {
		options.c_cc[VMIN] = numBytes; //set the minimum number of bytes with which to return
		tcsetattr(fdesc, TCSANOW, &options);
	}
	const int r = read(fdesc, buf, numBytes);
	if(r == 0) //EOF
		throw SerialException("EOF received on port (port closed)");
	if(r == -1)
		throw SerialException("cannot read from port");
	return r;
}

void nothing_handler(int signal) {}
/**
 * Send data across serial port.
 * @throw SerialException if unable to send all data
 * @param buf buffer from which to read data
 * @param numBytes number of bytes to send
 */
void SerialPort::send(const char* buf, size_t numBytes) throw (SerialException)
{
	if(write(fdesc, buf, numBytes) == -1)
		throw SerialException("cannot send to port");
	
	//The SIGALRM signal will interrupt tcdrain and prevent it from hanging the program. First save the
	//previous handler and alarm state (if any) so as to minimally disrupt the use of SIGALRM elsewhere.
	struct sigaction old, act;
		//sa_sigaction and sa_handler may be in a union, so we must set the one we want last so it want be overwritten.
		act.sa_sigaction = NULL; act.sa_handler = nothing_handler; sigemptyset(&act.sa_mask); act.sa_flags = 0; act.sa_restorer = NULL;
	sigaction(SIGALRM, &act, &old);
	unsigned int oldAlarm = alarm(1);
	
	bool bad = tcdrain(fdesc);
	
	alarm(oldAlarm);
	sigaction(SIGALRM, &old, NULL);
		
	if(bad)
		throw SerialException("error on serial port");
}

