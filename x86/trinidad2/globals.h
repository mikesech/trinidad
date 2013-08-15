#ifndef GLOBALS_H
#define GLOBALS_H

#include <cmath>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// Global Constants related to URT
const unsigned int WATCHDOG_TIMEOUT = 1000;
extern const unsigned int INTERVAL_TIMEOUT;
const unsigned short PORT = 4444;

#define _(x) x,(sizeof(x)-1)
const std::string DRIVE_MOTOR("_drive");
const std::string STEER_MOTOR("_steer");
const std::string COMPASS_KEY(_("3\0Compass"));
const std::string LATITUDE_KEY(_("1\0lat"));
const std::string LONGITUDE_KEY(_("1\0long"));
const std::string BUMPER_KEY(_("A\0bumper"));
const std::string SONAR_KEY(_("A\0sonarft"));
const std::string DEADMAN_KEY(_("A\0deadman"));
const std::string LM_12V_KEY("_+12V");
const std::string MOTOR_BATTERY_KEY("_motorv");
extern const int STEP_PWM;
extern const int MAX_DRIVE_PWM;
extern const int MAX_STEER_PWM;
extern const int MIN_DRIVE_PWM;
extern const int MIN_STEER_PWM;
extern const int NTL_PWM;
extern const int CONE_RANGE_FT;
extern const int DISTANCE_THRESHOLD_FT;
extern const int OBSTACLE_BREADTH_FT;
extern const double MIN_ALLOW_DISTANCE;
extern const int MAX_SONAR_SIGNAL;
extern const int BACKUP_DIST_FT;

// Global inline template functions
template <typename T>
inline T angularDifference(T x, T y) {
	T diffHeading = x - y;
	if(diffHeading < -180)
		diffHeading += 360;
	else if(diffHeading > 180)
		diffHeading -= 360;
	return diffHeading;
}
template <typename T>
inline T angularSum(T x, T y) {
	T sum = x + y;
	if(sum > 360)
		return sum - 360;
	else
		return sum;
}

template <typename T>
inline T northToEast(T northern) {
	return (northern <= 90)?(90 - northern):(450 - northern);
}


#endif
