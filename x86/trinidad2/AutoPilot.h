#ifndef _AUTOPILOT_H
#define _AUTOPILOT_H

#include "Waypoint.h"
#include <vector>
#include <ctime>
#include "camera.h"

class Camera;

class AutoPilot {
public:
	AutoPilot(const Waypoints& waypoints, Camera& camera)
		: waypoints(waypoints), camera(camera), state(INITIALIZING), curWaypoint(waypoints.begin()), coneOnPause(false) {}
	
	void realize();

private:
	enum RobotState {
		INITIALIZING,
		CRUISING,
		OBSTACLE_AVOID,
		HONING_ON_CONE,
		DISENGAGING_FROM_CONE,
		RECOVERING_FROM_COLLISION,
		DONE
	};
	
	enum AvoidMode {
		//LEFT = -1,
		//RIGHT = 1
		LEFT = 1,
		RIGHT = -1
	};

	// General Methods
	void adjustMotors(double angularOffset, int driveSpeed);
	void incrementWaypoint();
	
	// Pathfinding-Specific Methods
	double newHeading(double x, double y, double destinationX, double destinationY);
	
	// General Member Variables
	const Waypoints& waypoints;
	Camera& camera;
	RobotState state;
	Waypoints::const_iterator curWaypoint;
	double avoidanceHeading;
	timespec firstTime;
	timespec lastTime;
	timespec lastConeTime;
	timespec curConeTime;
	timespec firstPauseTime;
	timespec curPauseTime;
	double distTravelled;
	double obstacleDistance;
	double initObstacleAvoidanceHeading;
	AvoidMode mode;
	bool coneOnPause;

};

#endif
