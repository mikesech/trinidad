#ifndef _WAYPOINT_H
#define _WAYPOINT_H

#include <deque>

struct Waypoint {
	double latitude;
	double longitude;
	bool hasCone;
};

typedef std::deque<Waypoint> Waypoints;

#endif
