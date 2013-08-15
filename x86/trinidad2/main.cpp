/*
 * Trinidad is the next generation RoboServer designed for the RoboMagellan project. It is based on URT.
 */

#include "URT/EventLoop.h"
#include "URT/SocketServer.h"
#include "URT/State.h"
#include "URT/StateDevice.h"
#include "URT/StateSocket.h"
#include "URT/ExternalProgram.h"
#include "URT/Watchdog.h"
#include "URT/Log.h"
#include "URT/HotDeviceManager.h"
#include "URT/contrib/Ax3500.h"
#include "URT/contrib/LMSensors.h"

#include "globals.h"
#include "Waypoint.h"
#include "Parameters.h"
#include "AutoPilot.h"
#include "screen.h"
#include "camera.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <boost/bind.hpp>
#include <signal.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>

std::string formatTime() {
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	time_t now_time = now.tv_sec;
	char* theTime = ctime(&now_time);
	theTime[strlen(theTime)-1] = '\0'; //get rid of newline at end
	
	std::stringstream ss;	
	ss<<theTime<<" +"<<std::setfill('0')<<std::setw(3)<<now.tv_nsec/1000000<<"ms";
	return ss.str();
}

void printStats() {
try {
	const std::string time = formatTime();
	urt::Log::msg<<"\n\n"<<"Time: "<<time;
	updateStat("Time", time);
	updateLog("Drive motor", urt::State::get(DRIVE_MOTOR));
	updateLog("Steer motor", urt::State::get(STEER_MOTOR));
	const double northern = urt::State::getAs<int>(COMPASS_KEY)/10.0;
	const double present = (northern <= 90)?(90 - northern):(450 - northern);
	updateLog("Current heading", present);;
	updateLog("Latitude", urt::State::get(LATITUDE_KEY));
	updateLog("Longitude", urt::State::get(LONGITUDE_KEY));
	updateLog("UTC", urt::State::get(std::string("1\0utc",5)));
	updateLog("HDOP (m)", urt::State::getAs<double>(std::string("1\0hDilution",11)) * 6);
	updateLog("Sonar (ft.)", urt::State::get(SONAR_KEY));
	updateLog("Computer +12V", urt::State::get(LM_12V_KEY));
	//updateLog("Motor battery", urt::State::get(MOTOR_BATTERY_KEY));
	
	urt::Log::msg.flush();
} catch(...) {}
}

std::ostream& urt::Log::msg = getAutoLog();
std::ostream& urt::Log::warn = getAutoLog();
std::ostream& urt::Log::err = getAutoLog();

void deadman(const std::string& key, const std::string& value) {
	static int drive = NTL_PWM, steer = NTL_PWM;
	const bool d = !urt::State::getAs<bool>(DEADMAN_KEY);

	if (d) {
		drive = urt::State::getAs<int>(DRIVE_MOTOR);
		steer = urt::State::getAs<int>(STEER_MOTOR);
		urt::State::set(DRIVE_MOTOR, NTL_PWM);
		urt::State::set(STEER_MOTOR, NTL_PWM);
	} else {
		urt::State::set(DRIVE_MOTOR, drive);
		urt::State::set(STEER_MOTOR, steer);
	}
	updateLog("Dead man's switch", d);
}

int main(int argc, char* argv[])
{
	//enable maximum core dumps
	struct rlimit rl;
	getrlimit(RLIMIT_CORE, &rl);
	rl.rlim_cur = rl.rlim_max;
	setrlimit(RLIMIT_CORE, &rl);

try {
	//Load waypoints
	Waypoints waypoints;
	if(!loadConfiguration(waypoints, argc, argv))
		return 1;
		
	//setup screen
	ScreenGuard sg;
	
	//We have to ignore SIGWINCH
	//so it doesn't screw up our signal-ignorant syscalls in URT.
	//This isn't ideal, because ncurses will not adjust to a resize at all.
	sigset(SIGWINCH, SIG_IGN);
		
	urt::Log::msg<<"Configuration loaded:";
	for(Waypoints::iterator i = waypoints.begin(); i != waypoints.end(); i++)
		urt::Log::msg<<"\n\tWaypoint "<<(i - waypoints.begin() + 1)<<": "<<i->latitude<<", "<<i->longitude<<'\t'<<((i->hasCone)?("Cone"):("No Cone"));
	urt::Log::msg<<"\n\n";
	
	//Initialize camera
	urt::Log::msg<<"Initalizing camera...";
	Camera cam;
	urt::Log::msg<<" done.\n";

	//Start URT subsystem
	urt::Log::message("Starting Trinidad");
	urt::EventLoop loop(INTERVAL_TIMEOUT);

	//Setup deadman switch
	urt::State::registerSlot(DEADMAN_KEY, deadman);
	
	//Watch +12V from lmsensors
	try {
		loop.registerIntervalSlot(urt::contrib::LMSensors::getSingleton().substateUpdater("+12V", LM_12V_KEY));
	} catch (urt::contrib::LMSensors::Error& e) {
		urt::Log::warn<<"Warning: cannot activate LMSensors module. "<<e.what()<<'\n';
	}
	
	//Create objects for all StateDevices as well as the Ax3500
	{ //Pointer should not really be used after adding to loop. We'll make it go out of scope.
	urt::HotDeviceManager* dm = new urt::HotDeviceManager(loop);
	dm->addDirectory("/dev", "ttyUSB[[:digit:]]+");
	dm->execute(loop);
	loop.add(dm);
	
	switch(dm->getFailed().size()) {
	case 0:
		urt::Log::error("Ax3500 not plugged in! Aborting.");
		return 1;
	case 1: {
		//Assume the failed device is an Ax3500.
		urt::Log::msg<<"Potential Ax3500 candidate at "<<dm->getFailed().front()<<'\n';
		urt::contrib::Ax3500* ax3500 = new urt::contrib::Ax3500(dm->getFailed().front().c_str());
		loop.add(ax3500);
		//For some reason, our robot's current configuration is such that the channels are reversed.
		urt::State::registerSlot(STEER_MOTOR, &urt::contrib::Ax3500::setLinearSpeed, *ax3500);
		urt::State::registerSlot(DRIVE_MOTOR, &urt::contrib::Ax3500::setSteeringSpeed, *ax3500);
		//No watchdog reset is necessary since we'll be asking for the battery voltage with enough frequency.
		//ax3500->registerBatteryVoltage(MOTOR_BATTERY_KEY);
		//loop.registerIntervalSlot(&urt::contrib::Ax3500::requestBatteryVoltage, *ax3500);
		loop.registerIntervalSlot(&urt::contrib::Ax3500::resetWatchdog, *ax3500);
		break;
	}
	default:
		urt::Log::error("Too many failed devices! Aborting.");
		return 1;
	}
	}
	
	//Activate SocketServer to enable watching remotely (although ssh may be superior).
	loop.add(new urt::SocketServer<urt::StateSocket>(PORT,&loop));
	urt::Log::msg<<"All devices loaded.\nListening on port "<<PORT<<".\nInitializing automation systems."<<std::endl;
	
	//Create AutoPilot
	AutoPilot autopilot(waypoints,cam);
	loop.registerIntervalSlot(boost::bind(&AutoPilot::realize, &autopilot));
	
	//Setup other stuff
	loop.registerIntervalSlot(printStats);
	
	loop.run();

	urt::Log::error("No event handlers exist. Terminating program.");
	return 0;
} catch (cv::Exception& e) {
	//Ncurses has been uninitialized because the ScreenGuard is in the try
	//block. Plus, the message would disappear the minute ncurses resets
	//the terminal. So, we write to cerr and not urt::Log::err.
	std::cerr<<"Failure loading camera. Aborting.\n";
	//If cerr is not a terminal, print to cout as well since it may be a
	//terminal and the user probably won't think to look in the stderr
	//output to find out why the program did nothing and exited.
	if(!isatty(STDERR_FILENO))
		std::cout<<"Failure loading camera. Aborting.\n";
	return 1;
}
}
