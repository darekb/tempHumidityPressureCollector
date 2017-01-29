#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define showDebugDataMain 0

#include "main.h"
#include "slI2C.h"

#if showDebugDataMain == 1
#include "slUart.h"
#endif

#include "BME280.h"
#include "VirtualWire.h"

//TODO 1. wgrać do innego uC wsad z nasłuchiwaniem RF module 433.92MHz
//TODO 2. wywalać do uart surowe wartości
//TODO 3. wymienić uC na ten zaprogramowany przezemnie
//TODO 4. merge to master (WIN!!!)

//deploy checklist
//1. ustawić showDebugDataMain na 0
//2. ustawić w bibliotekach logowanie na 0
//3. usunąć z CMakeLists.txt slUart.c

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED
#define DELIMITER "|"

float temperature, humidity, pressure;
char req[39] = "";
volatile uint8_t stage = 0;
volatile uint16_t counter = 0;

void toStringToSend(float inData, char *out) {
  char bufor[10] = "";
  dtostrf((float) inData, 1, 2, bufor);
  strcat(out, bufor);
  strcat(out, DELIMITER);
}

uint8_t stage1_SetBME280Mode() {
  if (BME280_SetMode(BME280_MODE_FORCED)) {
#if showDebugDataMain == 1
    slUART_WriteString("Sensor set forced mode error!\r\n");
#endif
    return 1;
  }
  return 0;
}

uint8_t stage2_GetDataFromBME280() {
  if (BME280_ReadAll(&temperature, &pressure, &humidity)) {
#if showDebugDataMain == 1
    slUART_WriteString("Sensor read error!\r\n");
#endif
    return 1;
  }
  return 0;
}

uint8_t stage3_prepareDataToSend() {
  strcpy(req, "");
  //temperature
  toStringToSend(temperature, req);
  //humidity
  toStringToSend(humidity, req);
  //pressure
  toStringToSend(pressure, req);
  //vcc
  float vcc = 3.27;
  toStringToSend(vcc, req);
  //sensor nr an char endig transmission
  strcat(req, "11|z");
  return 0;
}
uint8_t stage4_sendViaRadio() {
  LED_TOG;
  //strcpy(req, "25.89|43.58|102410.00|0|z");
  vw_send((uint8_t *) req, strlen(req));
  vw_wait_tx(); // Wait until the whole message is gone
#if showDebugDataMain == 1
  slUART_WriteString("Send value: ");
  slUART_WriteString(req);
  slUART_WriteString("\r\n");
#endif
  LED_TOG;
  return 0;
}


int main(void) {
  TCCR0B |= (1 << CS02) | (1 << CS00);//prescaler 1024
  TIMSK0 |= (1 << TOIE0);//przerwanie przy przepłnieniu timera0

  DDRB |= LED;

  slI2C_Init();

#if showDebugDataMain == 1
  slUART_SimpleTransmitInit();
#endif

  vw_setup(2000);   // Bits per sec
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
    switch (stage) {
      case 1:
#if showDebugDataMain == 1
        slUART_WriteString("Stage1.\r\n");
#endif
        if (stage1_SetBME280Mode()) {
          stage = 0;
#if showDebugDataMain == 1
          slUART_WriteString("Error stage 1; stage1_SetBME280Mode().\r\n");
#endif
          return 1;
        }
        stage = 2;
        break;
      case 2:
#if showDebugDataMain == 1
        slUART_WriteString("Stage2.\r\n");
#endif
        if (stage2_GetDataFromBME280()) {
          stage = 0;
          return 1;
        }
        stage = 3;
        break;
      case 3:
#if showDebugDataMain == 1
        slUART_WriteString("Stage3.\r\n");
#endif
        if (stage3_prepareDataToSend()) {
          stage = 0;
          return 1;
        }
        stage = 4;
        break;
      case 4:
#if showDebugDataMain == 1
        slUART_WriteString("Stage4.\r\n");
#endif
        if (stage4_sendViaRadio()) {
          stage = 0;
          return 1;
        }
        stage = 0;
        break;
    }
  }
  return 0;
}

ISR(TIMER0_OVF_vect) {
  //co 0.01632sek.
  counter = counter + 1;
  if (counter == 306) {//9.98784 sek
    counter = 0;
#if showDebugDataMain == 1
    slUART_WriteString("Counter tick.\r\n");
#endif
    if(stage == 0){
#if showDebugDataMain == 1
      slUART_WriteString("Change stage value.\r\n");
#endif
      stage = 1;
    }
  }
}