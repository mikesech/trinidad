//Compass & Gyro Module for the URT

#include "MiniURT/miniurt.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ardlib/ardlib.h"
#include "ardlib/cmps03.h"
#include "ardlib/i2c.h"

const unsigned char APPLICATION_TYPE = '3';
const unsigned char UID = 0;

const uint16_t MIN_VELOCITY_TIME = F_CPU/640; //do calculation every .1 seconds

double angularVelocity;
int16_t compass;
uint8_t accx, accy, accz;

inline void onPoll(void) 
{
	char string[11];
	setSubstate(_("Compass"), itoa(compass, string, 10), strlen(string)); //Send Compass 
	//setSubstate(_("avelocity"), dtostre(angularVelocity, string, 3, 0), strlen(string)); //Send Angular Velocity 
	setSubstate(_("avelocity"), dtostrf(angularVelocity, 3, 3, string), strlen(string)); //Send Angular Velocity 
	//setSubstate(_("AccX"), utoa(accx, string, 10), strlen(string)); //Send AccX
	//setSubstate(_("AccY"), utoa(accy, string, 10), strlen(string)); //Send AccY
	//setSubstate(_("AccZ"), utoa(accz, string, 10), strlen(string)); //Send AccZ
}

inline int16_t angleDifference(int16_t x, int16_t y) {
	int16_t diff = x - y;
	if(diff < -1800)
		diff += 3600;
	else if(diff > 1800)
		diff -= 3600;
	return diff;
}

int main(void) {
	//adc_init();
	i2c_init();
	
	//initialize timer
	TCCR1A = 0b00000000; // normal counter (0 to TOP = 0xFFFF)
	TCCR1B = 0b00000011; // prescaler to 64
	
	//initialize MiniURT library
	initialize_baud(19200);
	
	while(1) {
		static int16_t oldCompass;
		compass = cmps03_direction();
		uint16_t time = TCNT1;
		if(time >= MIN_VELOCITY_TIME) {
			TCNT1 = 0;
			angularVelocity = angleDifference(compass, oldCompass)/(((double)time)/(F_CPU/64))/10.0;
			oldCompass = compass;
		}
		
		//accx = adc_read(0);
		//accy = adc_read(1);
		//accz = adc_read(2);
		
		if(polled())
			onPoll();
	}
}

void onInit(void) {}
void onIncoming(Substate* s) {}
