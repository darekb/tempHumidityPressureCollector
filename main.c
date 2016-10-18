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


//#define WDIFr
#ifdef WDIFr
   static void __init3( void ) __attribute__ (( section( ".init3" ), naked, used ));
   static void __init3( void )
   {
       /* wyłączenie watchdoga (w tych mikrokontrolerach, w których watchdog
        * ma możliwość generowania przerwania pozostaje on też aktywny po
        * resecie) */
       MCUSR = 0;
       WDTCSR = (1<<WDCE) | (1<<WDE);
       WDTCSR = 0;
   }
#endif

//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

void testDiv(void){
  uint32_t wynik = 1234;
  char *t1, *t2;
  div_t divmod = div(wynik, 100);
  itoa(divmod.quot, t1, 10);
  itoa(divmod.rem, t2, 10);
  slUART_WriteString("Test itoa t1: ");
  slUART_WriteString(t1);
  slUART_WriteString(" t2: ");
  slUART_WriteString(t2);
  slUART_WriteString("\r\n");
}

int main(void) {
  DDRB |= LED;
  int32_t temperature;
  uint32_t humidity;
  float pressure;
  char req[100];
  slI2C_Init();
  slUART_SimpleTransmitInit();
#ifdef TEST
  slUART_WriteString("test Cmake.\r\n");
  slUART_WriteString(TEST);
  slUART_WriteString("\r\n");
#endif
  slUART_WriteString("Start.\r\n");
  char t2[100];
  float test = ((float)1/3)*1000;
  sprintf(t2,"test: %u", test);
  slUART_WriteString(t2);
  slUART_WriteString("\r\n");

  //from BME280 lib
  //if (BME280_Init(BME280_OS_T_16, BME280_OS_P_16, BME280_OS_H_16, BME280_FILTER_16, BME280_MODE_NORMAL, BME280_TSB_62)) {

  //from Bosch Sensortec lib
  //if (BME280_Init(BME280_OS_T_4, BME280_OS_P_2, BME280_OS_H_1, BME280_FILTER_OFF, BME280_MODE_NORMAL, BME280_TSB_05)) {

  //from documentation for weather measurment
  if (BME280_Init(BME280_OS_T_1, BME280_OS_P_1, BME280_OS_H_1, BME280_FILTER_OFF, BME280_MODE_FORCED, BME280_TSB_1000)) {
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

    //dtostrf((double)(pressure, 9, 3, req);
    sprintf(req, "Pres: %u \r\n", pressure);
    slUART_WriteString(req);
//    sprintf(req, "Temp: %d.%02u Hum: %u.%02u Press: %u",
//            temperature / 100, temperature % 100, //C
//        //(pressure >> 8), ((pressure & 0x000000FF) * 100) >> 8,//Pa
//            humidity /100, humidity % 100,//rH
//            //humidity >> 10, ((humidity & 0x000003FF) * 100) >> 10,//rH
//            //(pressure >> 8) / 100, (pressure >> 8) % 100 //hPa
//            pressure //hPa
//            //humidity /100, humidity % 100//rH
//    );
//    slUART_WriteString(req);
//    //slUART_LogDec(temperature);
//    slUART_WriteString("\r\n");
    LED_TOG;
    _delay_ms(5000);
  }
  return 0;
}