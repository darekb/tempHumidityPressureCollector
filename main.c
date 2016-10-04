#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "slI2C.h"
#include "slUart.h"

#TODO add BME280 library
#TODO pomiary na rządanie
#TODO 
#TODO średnia wleczone dla pomiarów
#TODO implementacja RF module 433.92MHz

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
