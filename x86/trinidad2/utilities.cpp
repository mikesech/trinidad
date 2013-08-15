#include "utilities.h"
#include <cmath>

using namespace std;

double Distance(double x1, double y1, double x2, double y2)
{	
	double R = 20925524.9; // radius of earth in feet
	double dLat = (y2-y1)*M_PI/180;  // change in latitude in radians
	double dLon = (x2-x1)*M_PI/180;  // change in longitude in radians
	double a = sin(dLat/2) * sin(dLat/2) + cos(y1*M_PI/180) * cos(y2*M_PI/180) * sin(dLon/2) * sin(dLon/2); 
	double c = 2 * atan2(sqrt(a), sqrt(1-a)); 
	return R * c;	
}

bool withinDistance(double x1, double y1, double x2, double y2, double distance)
{
	if (Distance(x1, y1, x2, y2) <= distance)
		return true;
	return false;
}



bool withinAngle(double angle1, double angle2, double angleMargin)
{
	if (abs(angle2 - angle1) <= angleMargin || abs(angle1 + 2*M_PI - angle2) <= angleMargin || abs(angle2 + 2*M_PI - angle1) <= angleMargin)
		return true;
	return false;
}

