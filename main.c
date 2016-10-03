#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "slUart.h"


#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

int main(void) {
	slUART_SimpleTransmitInit();
	DDRB |= LED;


	while (1) {

		LED_TOG;
		_delay_ms(5000);
	}
}
