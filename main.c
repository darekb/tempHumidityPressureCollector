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

//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

uint8_t sendData(){
  float temperature, humidity, pressure;
  char req[100] = "";
  char bufor[5] = "";

  if (BME280_SetMode(BME280_MODE_FORCED)) {
    slUART_WriteString("Sensor set forced mode error!\r\n");
    return 1;
  }
  if (BME280_ReadAll(&temperature, &pressure, &humidity)) {
    slUART_WriteString("Sensor read error!\r\n");
    return 1;
  }

  strcat(req,"Temp: ");
  dtostrf((float)temperature, 1, 2, bufor);
  strcat(req, bufor);

  strcat(req,"°C Hum: ");
  dtostrf((float)humidity, 1, 2, bufor);
  strcat(req, bufor);

  strcat(req,"% Press: ");
  dtostrf((float)pressure, 1, 2, bufor);
  strcat(req, bufor);

  slUART_WriteString(req);
  slUART_WriteString("Pa\r\n");

  return 0;
}

int main(void) {
  DDRB |= LED;
  slI2C_Init();
  slUART_SimpleTransmitInit();
  slUART_WriteString("Start.\r\n");

  if (BME280_Init(BME280_OS_T_1, BME280_OS_P_1, BME280_OS_H_1, BME280_FILTER_OFF, BME280_MODE_FORCED, BME280_TSB_1000)) {
    slUART_WriteString("BMP280 init error.\r\n");
  } else {
    slUART_WriteString("BMP280 init done.\r\n");
  }
  while (1) {
    LED_TOG;
    _delay_ms(100);
    if(sendData()){
      slUART_WriteString("Send data error.\r\n");
    }
    LED_TOG;
    _delay_ms(5000);
  }
  return 0;
}