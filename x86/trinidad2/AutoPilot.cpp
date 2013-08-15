#include "AutoPilot.h"
#include "globals.h"
#include "utilities.h"
#include "URT/State.h"
#include "URT/Log.h"
#include <ctime>
#include <cmath>
#include "screen.h"
#include "camera.h"
#include <limits>

//////////////// Helper Functions
static double nemaSpaceToDegrees(const std::string& in) {
	size_t dot = in.rfind('.');
	if(dot == std::string::npos || dot < 2)
		throw std::runtime_error("invalid NEMA_space string");
	size_t degLength = dot - 2;

	double degrees;

	/*Convert degrees*/ {
		stringstream ss(in.substr(0, degLength));
		ss >> degrees;
	}

	/*Convert minutes, get direction*/ {
		stringstream ss(in.substr(degLength));
		double minutes;
		ss >> minutes;

		degrees += minutes/60;

		char dir;
		ss >> dir;
		if(dir == 'W' || dir =='S')
			return -degrees;
		else
			return degrees;
	}
}
static inline double getLong() {
	return nemaSpaceToDegrees(urt::State::get(LONGITUDE_KEY));
}
static inline double getLat() {
	return nemaSpaceToDegrees(urt::State::get(LATITUDE_KEY));
}

/////////////// General Class Methods
void AutoPilot::realize() {
	try {
	if(!urt::State::getAs<bool>(DEADMAN_KEY)) {
		return;
	}
	} catch(...) {
		return;
	}


evaluate:
	switch(state) {
		case INITIALIZING: {
			updateLog("AutoPilot", "initializing systems");
			urt::State::set(DRIVE_MOTOR, NTL_PWM);
			urt::State::set(STEER_MOTOR, NTL_PWM);

			state = CRUISING;
			//updateLog("AutoPilot", "cruising");
		} break;
		case HONING_ON_CONE: try {
			if (urt::State::getAs<bool>(BUMPER_KEY)) {
				state = DISENGAGING_FROM_CONE;
				clock_gettime(CLOCK_MONOTONIC, &firstTime);
				incrementWaypoint();
				goto evaluate;
			}

			if (coneOnPause) {
				clock_gettime(CLOCK_MONOTONIC, &curPauseTime);
				double timeLapse = curPauseTime.tv_sec - firstPauseTime.tv_sec + (curPauseTime.tv_nsec - firstPauseTime.tv_nsec)/1e9;
				if (timeLapse > 1) {
					coneOnPause = false;
					break;
				}
				else {
					adjustMotors(0,0);
					break;
				}
			}

			double currentLong = getLong();
			double currentLat = getLat();
			double present = northToEast(urt::State::getAs<int>(COMPASS_KEY)/10.0);
			double desired = newHeading(currentLong, currentLat, curWaypoint->longitude, curWaypoint->latitude);
			bool coneExists = 0;


//			Camera camera;
			cvReleaseCapture(&camera.capture);
			camera.capture = cvCaptureFromCAM(0);
			double camHeading = camera.getCameraHeading(coneExists);


			if (!coneExists) {

				//adjustMotors(angularDifference(desired, present), MAX_DRIVE_PWM*0.7);
				adjustMotors(50,0);
				updateLog("AutoPilot", "honing on cone (no cone)");
			}
			else {
				clock_gettime(CLOCK_MONOTONIC, &curConeTime);
				double timeDiff = curConeTime.tv_sec - lastConeTime.tv_sec + (curConeTime.tv_nsec - lastConeTime.tv_nsec)/1e9;
				if (timeDiff > 2) {
					coneOnPause = true;
					clock_gettime(CLOCK_MONOTONIC, &firstPauseTime);
					lastConeTime = curConeTime;
				}
				else {
					lastConeTime = curConeTime;
					desired = present - camHeading;
					adjustMotors(2*angularDifference(desired, present), MAX_DRIVE_PWM*0.5);
					updateLog("AutoPilot", "honing on cone (cone)");
				}
			}

			updateLog("Desired heading", desired);
			break;
		} catch(...) {} break;
		case CRUISING: try {
			updateLog("AutoPilot", "cruising");
			double currentLong = getLong();
			double currentLat = getLat();
			double present = northToEast(urt::State::getAs<int>(COMPASS_KEY)/10.0);
			double desired = newHeading(currentLong, currentLat, curWaypoint->longitude, curWaypoint->latitude);
			updateLog("Desired heading", desired);

			double curSonar = urt::State::getAs<double>(SONAR_KEY);

			if (curSonar < DISTANCE_THRESHOLD_FT && fabs(angularDifference(desired,present)) < 45) {
				state = OBSTACLE_AVOID;
				initObstacleAvoidanceHeading = present;
				mode = RIGHT;
				goto evaluate;
			}

			adjustMotors(2*angularDifference(desired, present), (curSonar/MAX_SONAR_SIGNAL)*(MAX_DRIVE_PWM - fabs(angularDifference(desired,present))*MAX_DRIVE_PWM/180.0));

			if(withinDistance(currentLong, currentLat, curWaypoint->longitude, curWaypoint->latitude, CONE_RANGE_FT) && curWaypoint->hasCone) {
				clock_gettime(CLOCK_MONOTONIC, &lastConeTime);
				state = HONING_ON_CONE;
				goto evaluate;
			} else if(withinDistance(currentLong, currentLat, curWaypoint->longitude, curWaypoint->latitude, CONE_RANGE_FT)) {
				incrementWaypoint();
			}
		} catch(...) {} break;
		case OBSTACLE_AVOID: {
			double present = northToEast(urt::State::getAs<int>(COMPASS_KEY)/10.0);
			if (abs(angularDifference(present,initObstacleAvoidanceHeading)) > 125)
				mode = LEFT;

			if(mode == LEFT)
				updateLog("AutoPilot", "avoiding obstacle (left)");
			else if(mode == RIGHT)
				updateLog("AutoPilot", "avoiding obstacle (right)");

			double curSonar = urt::State::getAs<double>(SONAR_KEY);
			if (curSonar < DISTANCE_THRESHOLD_FT) {
				avoidanceHeading = present - mode*atan2(OBSTACLE_BREADTH_FT, curSonar)*180/M_PI;
				clock_gettime(CLOCK_MONOTONIC, &lastTime);
				distTravelled = 0;
				obstacleDistance = curSonar;
			}

			int driveNumber = urt::State::getAs<int>(DRIVE_MOTOR);
			double speed = -driveNumber/127.0*330.729166666667*2*M_PI*7/12/60; // in feet/second

			timespec curTime;
			clock_gettime(CLOCK_MONOTONIC, &curTime);

			double timelapse = curTime.tv_sec - lastTime.tv_sec + (curTime.tv_nsec - lastTime.tv_nsec)/1e9;

			distTravelled += speed*timelapse;
			updateLog("Distance travelled", distTravelled);
			updateLog("Obstacle distance", obstacleDistance);

			if (distTravelled > obstacleDistance)
			{
				updateLog("AutoPilot", "obstacle avoided; resuming cruising");
				state = CRUISING;
				goto evaluate;
			}

			lastTime = curTime;
			adjustMotors(angularDifference(avoidanceHeading, present), (curSonar - MIN_ALLOW_DISTANCE)*(MAX_DRIVE_PWM)/(MAX_SONAR_SIGNAL - MIN_ALLOW_DISTANCE));
		} break;
		case RECOVERING_FROM_COLLISION: {
			state = CRUISING;
		} break;
		case DISENGAGING_FROM_CONE: {
			timespec curTime;
			clock_gettime(CLOCK_MONOTONIC, &curTime);

			double timeDiff = curTime.tv_sec - firstTime.tv_sec + (curTime.tv_nsec - firstTime.tv_nsec)/1e9;
			int driveNumber = urt::State::getAs<int>(DRIVE_MOTOR);
			double speed = -driveNumber/127.0*330.729166666667*2*M_PI*7/12/60; // in feet/second

			if (fabs(speed*timeDiff) > BACKUP_DIST_FT) {
				state = CRUISING;
				goto evaluate;		
			}
			updateLog("AutoPilot", "disengaging from cone");

			adjustMotors(27, -5);
		} break;
		case DONE: {
		} break;
	}

	urt::Log::msg.flush();
}

void AutoPilot::incrementWaypoint() {
	urt::State::set(DRIVE_MOTOR, NTL_PWM);
	urt::State::set(STEER_MOTOR, NTL_PWM);
	if(++curWaypoint == waypoints.end()) {
		updateLog("AutoPilot", "program complete");;
		state = DONE;
	} else {
		updateLog("AutoPilot", "cruising to next waypoint");
	}
}

void AutoPilot::adjustMotors(double angularOffset, int driveSpeed) {
	int steer = -MAX_STEER_PWM*angularOffset/180;

	if (steer > MAX_STEER_PWM)
		steer = MAX_STEER_PWM;
	if (steer < -MAX_STEER_PWM)
		steer = -MAX_STEER_PWM;
	if (driveSpeed > MAX_DRIVE_PWM)
		driveSpeed = MAX_DRIVE_PWM;
	if (driveSpeed < -MAX_DRIVE_PWM)
		driveSpeed = -MAX_DRIVE_PWM;

	urt::State::set(DRIVE_MOTOR, -driveSpeed);
	urt::State::set(STEER_MOTOR, steer);
}

/////////////// Pathfinding-Specific Class Methods
double AutoPilot::newHeading(double x, double y, double destinationX, double destinationY) {
	double deltaX = destinationX - x;
	double deltaY = destinationY - y;

	double ret = atan2(deltaY, deltaX) * 180 / M_PI;
	if(ret < 0)
		return ret + 360;
	else
		return ret;
}

