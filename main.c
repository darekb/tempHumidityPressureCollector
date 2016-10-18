#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


#include "main.h"
#include "slI2C.h"
#include "slUart.h"
#include "BME280.h"


#define WDIFr
#ifdef WDIFr

static void __init3(void) __attribute__ (( section( ".init3" ), naked, used ));

static void __init3(void) {
  /* wyłączenie watchdoga (w tych mikrokontrolerach, w których watchdog
   * ma możliwość generowania przerwania pozostaje on też aktywny po
   * resecie) */
  MCUSR = 0;
  WDTCSR = (1 << WDCE) | (1 << WDE);
  WDTCSR = 0;
}

#endif

//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

int main(void) {
  DDRB |= LED;
  int32_t temperature;
  uint32_t humidity;
  float pressure;
  char req[100];
  slI2C_Init();
  slUART_SimpleTransmitInit();
  slUART_WriteString("Start.\r\n");

  if (BME280_Init(BME280_OS_T_1, BME280_OS_P_1, BME280_OS_H_1, BME280_FILTER_OFF, BME280_MODE_FORCED,
                  BME280_TSB_1000)) {
    slUART_WriteString("BMP280 init error.\r\n");
  } else {
    slUART_WriteString("BMP280 init done.\r\n");
  }
  while (1) {
    LED_TOG;
    _delay_ms(100);
    if (BME280_SetMode(BME280_MODE_FORCED)) {
      slUART_WriteString("Sensor set forced mode error!\r\n");
      return 1;
    }
    if (BME280_ReadAll(&temperature, &pressure, &humidity)) {
      slUART_WriteString("Sensor read error!\r\n");
      return 1;
    }
    sprintf(req, "Temp: %d ", temperature);
    slUART_WriteString(req);

    sprintf(req, "Hum: %u ", humidity);
    slUART_WriteString(req);

    sprintf(req, "Pres: %u \r\n", pressure);
    slUART_WriteString(req);
    LED_TOG;
    _delay_ms(5000);
  }
  return 0;
}