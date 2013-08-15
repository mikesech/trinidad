#ifndef _GPS_H_
#define _GPS_H_

#define GPS_BAUD 4800
#define GPSRATE (F_CPU / 16 / GPS_BAUD - 1)

void gps_init(void);
char* gps_getstring(char* outstring);

#endif
