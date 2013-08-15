#include "PURT/statedevice_buffer.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdbool.h>

const unsigned char APPLICATION_TYPE = 'A';
const unsigned char UID = 0;
double distanceArray[10] = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25};
double sum = 25*10;
#define BAUD 19200

static inline void activateEINT() {
       EIMSK = 0b00000011;
}
static inline void deactivateEINT() {
       EIMSK = 0;
}

void onPoll() {
	double copy_sum;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		copy_sum = sum;
	}

	const double average = copy_sum/10;
	
	char sonar_str[16];
	//Chris Wasson says 6 ought to be good enough for anyone!
	dtostre(average, sonar_str, 6, 0);
	
	//since the INT0 handler uses the serial port output, we have
	//to disable it while we transmit
    deactivateEINT(); //prevent reentrancy
	purt_set_substate(_("sonarft"), sonar_str, strlen(sonar_str));
	activateEINT();
}
void onHandshake() {
	deactivateEINT();
	if(PIND & 0b00000100)
		purt_set_substate(_("bumper"), _("0")); //logical high means no contact
	else
		purt_set_substate(_("bumper"), _("1")); //logical low means contact
		
	if(PIND & 0b00001000)
		purt_set_substate(_("deadman"), _("0")); //logical high means no contact
	else
		purt_set_substate(_("deadman"), _("1")); //logical low means contact
	activateEINT();
}
void onSubstate(Purt_Substate* s) {}

int main(void) {
	//activate timer 0 to trigger ADC every 50 ms
	TCCR1A = 0b00000000; //normal, ctc mode
	TCCR1B = 0b00011011; //ctc mode (ICR1A as top), prescalar @ 64
	ICR1 = 12500; //50 ms
	
	//activate ADC module
	DDRC &= 0b11111110; //set pin C1 for input
	ADMUX = (1<<REFS0); //use AVcc as reference; analog 0 as input
		//enable ADC, set auto-trigger, enable interrupt, prescalar @ 8	
	ADCSRA = (1<<ADEN) | (1<<ADATE) | (1<<ADIE) | ((1<<ADPS1) | (1<<ADPS0));
	ADCSRB = 0b111; //trigger on Timer 1 input capture
	
	//activate bumper
	DDRD &= 0b00001100; //set for input
	PORTD |= 0b00001100; //activate pull-up
	EICRA = 0b00000101; //logical change on INT0, INT1
	activateEINT(); //enable interrupt for INT0, INT1s
	
	purt_sd_buffer_init_enc((F_CPU + BAUD * 8L) / (BAUD * 16L) - 1);
	sei();
	
	while(1) {
		purt_sd_buffer_process_message();
	}
}

ISR(ADC_vect) {
	//We don't want to prevent other interrupts (particuarlly the UART one) from firing.
	//However, we don't want to reenter this ISR, so we only allow most of the code in
	//an nonatomic block (where interrupts are enabled). Since the ADC won't start again
	//until at least the Timer 1 overflow flag is cleared, we know we won't reenter. To play
	//it safe, we clear the flag when interrupts are disabled and let the hardware automatically
	//reenable them when we finally leave the ISR.

	NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) {
		const double sonar = ADC/24.0;
	
		sum -= distanceArray[0];
		for (int i = 0; i < 9; i++) {
			distanceArray[i] = distanceArray[i+1];
		}
	
		distanceArray[9] = sonar;
		sum += sonar;
	}
	
	TIFR1 = 0b00100000; //clear Timer 1 input capture flag (important since ADC is triggered on rising edge only)
}

ISR(INT0_vect) {
    deactivateEINT(); //prevent reentrancy
	NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) {
		if(PIND & 0b00000100)
			purt_set_substate(_("bumper"), _("0")); //logical high means no contact
		else
			purt_set_substate(_("bumper"), _("1")); //logical low means contact
	}
	activateEINT(); //prevent reentrancy
}

ISR(INT1_vect) {
    deactivateEINT(); //prevent reentrancy
	NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) {
		if(PIND & 0b00001000)
			purt_set_substate(_("deadman"), _("0")); //logical high means no contact
		else
			purt_set_substate(_("deadman"), _("1")); //logical low means contact
	}
	activateEINT(); //prevent reentrancy
}
