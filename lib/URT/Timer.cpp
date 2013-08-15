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

#include "Timer.h"
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

using namespace urt;

void Timer::timerThread(sigval_t obj) {
	const Timer* const ptr = reinterpret_cast<Timer*>(obj.sival_ptr);
	write(ptr->writePipe, "", 1); //send null terminator
}
Timer::Timer(unsigned int millis, bool start) throw (TimerException) {
	//create inter-thread communication pipe
	int pipefd[2];
	if(pipe(pipefd) != 0 || fcntl(pipefd[0], F_SETFL, O_NONBLOCK) == -1)
		throw TimerException("Error creating timer pipe.");
	fdesc = pipefd[0];
	writePipe = pipefd[1];
	
	//initalize sigevent to control the timer event (i.e., make it call timerThread in another thread)
	struct sigevent evp;
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_signo = 0;
	evp.sigev_value.sival_ptr = this;
	evp.sigev_notify_function = &Timer::timerThread;
	
	//specific attributes of new timer thread
	pthread_attr_t attr;
	pthread_attr_init(&attr);
		//we don't need to join; non-joinable (detached) save resources
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		//the thread basically needs only a few bytes of stack memory (for the parameter, stack ptr., etc.)
		//16384 is PTHREAD_STACK_MIN
	pthread_attr_setstacksize(&attr, 16384);
	evp.sigev_notify_attributes = &attr;
	
	//create the timer
	if(timer_create(CLOCK_MONOTONIC, &evp, &m_timerid) == -1) {
		close(fdesc);
		close(writePipe);
		pthread_attr_destroy(&attr);
		throw TimerException("Error creating real-time timer primative.");
	}
		
	//set the timer interval
	m_its.it_interval.tv_sec = millis / 1000;
	m_its.it_interval.tv_nsec = (millis % 1000) * 1000000;
	try {
		if(start) Timer::start();
		else stop();
	} catch(...) {
		close(fdesc);
		close(writePipe);
		pthread_attr_destroy(&attr);
		timer_delete(&m_timerid);
		throw;
	}

	//release attribute resources
	pthread_attr_destroy(&attr);
}
Timer::~Timer() {
	timer_delete(m_timerid);
	close(fdesc);
	close(writePipe);
}
bool Timer::onActivity() {
	//We use 2 so that if there is only 1 (as should be the case during normal operations),
	//we'll know with just one read call.
	char dummy[2];
	while(read(fdesc, &dummy, sizeof(dummy)) > 1); //empty buffer
	return onTimeout();
}
void Timer::start() {
	m_its.it_value.tv_sec = m_its.it_interval.tv_sec;
	m_its.it_value.tv_nsec = m_its.it_interval.tv_nsec;
	if(timer_settime(m_timerid, 0, &m_its, NULL) == -1)
		throw TimerException("Error starting timer.");
}
void Timer::stop() {
	m_its.it_value.tv_sec = 0;
	m_its.it_value.tv_nsec = 0;
	if(timer_settime(m_timerid, 0, &m_its, NULL) == -1)
		throw TimerException("Error stopping timer.");
}
