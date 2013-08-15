#include "PURT/statedevice_buffer.h"
#include "gps.h"
#include <string.h>
#include <util/atomic.h>
#include <stdint.h>
#include <stdbool.h>

//$GPGGA,024625.00,3404.1728,N,11826.6609,W,1,05,1.8,146.3,M,-33.7,M,,0000*65

const unsigned char APPLICATION_TYPE = '1';
const unsigned char UID = 0;
#define BAUD 19200UL

void onPoll() {
	char buf[128];
	char *utc = buf, *latitude = buf, *longitude = buf, *hDilution = buf, *alt = buf;
	uint8_t utcSize = 0, latitudeSize = 0, longitudeSize = 0, hDilutionSize = 0, altSize = 0;
	
	//Get strings until we recieve applicable data string
	do {
		gps_getstring(buf);
	} while(strncmp(buf,"$GPGGA",6) != 0);
	
	//Parse into fields
	char *str = buf;
	for(str += 6; *str != ','; ++str) if(*str == 0) continue; //go to next field (UTC)
	utc = ++str;
	for(; *str != ','; ++str) if(*str == 0) continue; //go to next field (latitude)
	utcSize = str - utc;
	latitude = ++str;
	for(; *str != ','; ++str) if(*str == 0) continue; //go to next field (latitude direction)
	*str = ' '; //change comma delimiter to more sensible space
	for(++str; *str != ','; ++str) if(*str == 0) continue; //go to next field (longitude)
	latitudeSize = str - latitude;
	longitude = ++str;
	for(; *str != ','; ++str) if(*str == 0) continue; //go to next field (longitude direction)
	*str = ' '; //change comma delimiter to more sensible space
	for(++str; *str != ','; ++str) if(*str == 0) continue; //go to next field (fix quality)
	longitudeSize = str - longitude;
	for(++str; *str != ','; ++str) if(*str == 0) continue; //go to next field (# satellites)
	for(++str; *str != ','; ++str) if(*str == 0) continue; //go to next field (horizontial dilution of precision)
	hDilution = ++str;
	for(; *str != ','; ++str) if(*str == 0) continue; //go to next field (altitude)
	hDilutionSize = str - hDilution;
	alt = ++str;
	for(; *str != ','; ++str) if(*str == 0) continue; //go to next field (altitude unit)
	*str = ' '; //change comma delimiter to more sensible space
	for(++str; *str != ','; ++str) if(*str == 0) continue; //go to next field
	altSize = str - alt;
	
	//Update server
	purt_set_substate(_("utc"), utc, utcSize);
	purt_set_substate(_("lat"), latitude, latitudeSize);
	purt_set_substate(_("long"), longitude, longitudeSize);
	purt_set_substate(_("hDilution"), hDilution, hDilutionSize);
	purt_set_substate(_("alt"), alt, altSize);
}
void onHandshake(void) {}
void onSubstate(Purt_Substate* s) {}

int main(void) {
	gps_init();
	purt_sd_buffer_init_enc((F_CPU + BAUD * 8L) / (BAUD * 16L) - 1);
	
	while(1) {
		purt_sd_buffer_process_message();
	}
}

