#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


#include "main.h"
#include "slI2C.h"
#include "slUart.h"
#include "BME280.h"
#include "VirtualWire.h"

//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

#define showDebugDataMain 1

uint8_t sendData() {
  float temperature, humidity, pressure;
  char req[100] = "";
  char bufor[5] = "";
  char delimiter[1] = "|";

  if (BME280_SetMode(BME280_MODE_FORCED)) {
#if showDebugDataMain == 1
    slUART_WriteString("Sensor set forced mode error!\r\n");
#endif
    return 1;
  }
  if (BME280_ReadAll(&temperature, &pressure, &humidity)) {
#if showDebugDataMain == 1
    slUART_WriteString("Sensor read error!\r\n");
#endif
    return 1;
  }
  //temperature
  strcat(req, "");
  dtostrf((float) temperature, 1, 2, bufor);
  strcat(req, bufor);
  //humidity
  strcat(req, delimiter);
  dtostrf((float) humidity, 1, 2, bufor);
  strcat(req, bufor);
  //pressure
  strcat(req, delimiter);
  dtostrf((float) pressure, 1, 2, bufor);
  strcat(req, bufor);
  //vcc
  strcat(req, delimiter);
  strcat(req, "0");
  //sensor nr
  strcat(req, delimiter);
  strcat(req, "11");
  strcat(req, delimiter);
  strcat(req, "z");

  slUART_WriteString(req);
  slUART_WriteString("\r\n");
  vw_send((uint8_t *)req, strlen(req));
  vw_wait_tx(); // Wait until the whole message is gone
  return 0;
}

int main(void) {
  DDRB |= LED;
  slI2C_Init();
  slUART_SimpleTransmitInit();
  vw_setup(2000);
#if showDebugDataMain == 1
  slUART_WriteString("Start.\r\n");
#endif
  if (BME280_Init(BME280_OS_T_1, BME280_OS_P_1, BME280_OS_H_1, BME280_FILTER_OFF, BME280_MODE_FORCED,
                  BME280_TSB_1000)) {
#if showDebugDataMain == 1
    slUART_WriteString("BMP280 init error.\r\n");
#endif
  } else {
#if showDebugDataMain == 1
    slUART_WriteString("BMP280 init done.\r\n");
#endif
  }
  while (1) {
    LED_TOG;
    _delay_ms(100);
    if (sendData()) {
#if showDebugDataMain == 1
      slUART_WriteString("Send data error.\r\n");
#endif
    }
    LED_TOG;
    _delay_ms(5000);
  }
  return 0;
}