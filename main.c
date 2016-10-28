#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define showDebugDataMain 1

#include "main.h"
#include "slI2C.h"
#if showDebugDataMain == 1
#include "slUart.h"
#endif
#include "BME280.h"
#include "VirtualWire.h"

//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED
#define DELIMITER "|"


void toStringToSend(float inData, char *out) {
  char bufor[10] = "";
  strcat(out, DELIMITER);
  dtostrf((float) inData, 1, 2, bufor);
  strcat(out, bufor);
}

uint8_t sendData() {
  float temperature, humidity, pressure;
  char req[39] = "";
  char buf[VW_MAX_MESSAGE_LEN];
  char buflen = VW_MAX_MESSAGE_LEN;
  char wiad[39] = "";

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
  toStringToSend(temperature, req);
  //humidity
  toStringToSend(humidity, req);
  //pressure
  toStringToSend(pressure, req);
  //vcc
  toStringToSend(0.0, req);
  //sensor nr an char endig transmission
  strcat(req, "|11|z");

  vw_send((uint8_t *) req, strlen(req));
  vw_wait_tx(); // Wait until the whole message is gone
#if showDebugDataMain == 1
  slUART_WriteString("Send value: ");
  slUART_WriteString(req);
  slUART_WriteString("\r\n");
#endif
  _delay_ms(100);
  if (vw_get_message(buf, &buflen)) {
    int i;
    for (i = 0; i < buflen; i++) {
      wiad[i] = buf[i];
    }
#if showDebugDataMain == 1
    slUART_WriteString("Read value: ");
    slUART_WriteString(wiad);
    slUART_WriteString("\r\n");
#endif
  }
  return 0;
}

int main(void) {
  char delimiter[1] = "|";
  DDRB |= LED;
  slI2C_Init();
  slUART_SimpleTransmitInit();
  vw_set_ptt_inverted(1); // Required for DR3100
  vw_setup(2000);   // Bits per sec
  //vw_set_tx_pin(13);//PB5 set pin in VirtualWire_Config.h
  vw_rx_start();
  sei();
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
    _delay_ms(6000);
  }
  return 0;
}