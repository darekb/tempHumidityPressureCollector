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
#include "nrf24l01.h"

//TODO średnia wleczone dla pomiarów

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED
#define DELIMITER "|"

float temperature, humidity, pressure;
char req[32] = "";
volatile uint8_t stage = 0;
volatile uint16_t counter = 0;

void toStringToSend(float inData, char *out) {
  char bufor[10] = "";
  strcat(out, DELIMITER);
  dtostrf((float) inData, 1, 2, bufor);
  strcat(out, bufor);
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
  toStringToSend(0.0, req);
  //sensor nr an char endig transmission
  strcat(req, "|11|z");
  return 0;
}

uint8_t stage4_sendVianRF24L01(nRF24L01 *rf, uint8_t to_address[5]) {
  nRF24L01Message msg;
  LED_TOG;
  memcpy(msg.data, req, 32);
  //msg.data = req;
  msg.length = strlen((char *)msg.data) + 1;
  nRF24L01_transmit(rf, to_address, &msg);
  int success = nRF24L01_transmit_success(rf);
  while(!success){
    success = nRF24L01_transmit_success(rf);
  }
  if (success != 0){
    nRF24L01_flush_transmit_message(rf);
  }
#if showDebugDataMain == 1
  slUART_WriteString("Send value: ");
  slUART_WriteString(req);
  slUART_WriteString("\r\n");
#endif
  LED_TOG;
  return 0;
}
nRF24L01 *setup_rf(void);

volatile bool rf_interrupt = false;
volatile bool send_message = false;

int main(void) {
  TCCR0B |= (1 << CS02) | (1 << CS00);//prescaler 1024
  TIMSK0 |= (1 << TOIE0);//przerwanie przy przepłnieniu timera0
  DDRB |= LED;
  slI2C_Init();
  slUART_SimpleTransmitInit();
  
  uint8_t to_address[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };
  bool on = false;
  sei();
  nRF24L01 *rf = setup_rf();
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
        if (stage1_SetBME280Mode()) {
          stage = 0;
          return 1;
        }
        stage = 2;
        break;
      case 2:
        if (stage2_GetDataFromBME280()) {
          stage = 0;
          return 1;
        }
        stage = 3;
        break;
      case 3:
        if (stage3_prepareDataToSend()) {
          stage = 0;
          return 1;
        }
        stage = 4;
        break;
      case 4:
        if (stage4_sendVianRF24L01(rf, to_address)) {
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
  if (counter == 366) {//5,97312 sek
    counter = 0;
    if(stage == 0){
      stage = 1;
    }
  }
}


nRF24L01 *setup_rf(void) {
    nRF24L01 *rf = nRF24L01_init();
    rf->ss.port = &PORTB;
    rf->ss.pin = PB2;
    rf->ce.port = &PORTB;
    rf->ce.pin = PB1;
    rf->sck.port = &PORTB;
    rf->sck.pin = PB5;
    rf->mosi.port = &PORTB;
    rf->mosi.pin = PB3;
    rf->miso.port = &PORTB;
    rf->miso.pin = PB4;
    // interrupt on falling edge of INT0 (PD2)
    EICRA |= _BV(ISC01);
    EIMSK |= _BV(INT0);
    nRF24L01_begin(rf);
    return rf;
}