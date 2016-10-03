#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//#include "slUart.h"
#include "slAdc.h"
#include "slLCDI2C.h"
#include "slMCP3421.h"

TVOL accu1_vol, mcp_vol;

#define LED (1 << PB5)
#define LED_TOG PORTB ^= LED

int main(void) {
	slADC_init();
	slLCDI2C_Init();
	//slMCP3421_Init();
	//slUART_SimpleTransmitInit();
	uint16_t wynik = 0;
	//uint16_t wynik2 = 0;
	DDRB |= LED;

//TODO convert adc measure to iterrupts

	//dla mierzonego napięcia 0 do 12v
	accu1_vol.ref_v = 1100; //dla 6V 6*100
	accu1_vol.ref_adc = 574; //zapisujemy co mamy w wyniku z ADC dla 6V czyli połowy zakresu napięcia które chcemy mierzyc
	accu1_vol.idx = 0;
//	accu1_vol.ref_v = 1100; //dla 6V 6*100
//	accu1_vol.ref_adc = 574; //zapisujemy co mamy w wyniku z ADC dla 6V czyli połowy zakresu napięcia które chcemy mierzyc
//	accu1_vol.idx = 0;

	mcp_vol.ref_v = 6000; //dla 6V 6*1000
	mcp_vol.ref_adc = 494; //zapisujemy co mamy w wyniku z ADC dla 6V czyli połowy zakresu napięcia które chcemy mierzyc
	mcp_vol.idx = 0;

	slLCDI2C_Clear();
	slLCDI2C_SetCursor(0, 0);
	slLCDI2C_WriteString("Test123");
	slLCDI2C_SetCursor(8, 1);
	slLCDI2C_WriteString("0.0mA");

	while (1) {
		wynik = slADC_measure(PC0);
		//wynik2 = slMCP3421_ReadByte();
		slLCDI2C_SetCursor(8, 0);
		slLCDI2C_WriteString("          ");
		//slLCDI2C_WriteString(wynik);

		get_vol(wynik, &accu1_vol);
		//get_vol(wynik2, &mcp_vol);

		slLCDI2C_SetCursor(0, 1);
		slLCDI2C_WriteString("          ");
		slLCDI2C_SetCursor(0, 1);
		slLCDI2C_WriteString(accu1_vol.v1);
		slLCDI2C_WriteString(".");
		slLCDI2C_WriteString(accu1_vol.v2);
		slLCDI2C_WriteString("V");

		LED_TOG;
		_delay_ms(100);
	}
}
