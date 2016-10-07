#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


#include "slI2C.h"
#include "slUart.h"
#include "BME280.h"

//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

int main(void) {
    DDRB |= LED;
    int32_t temperature;
    uint32_t pressure, humidity;
    char req[100];
    slI2C_Init();
    slUART_SimpleTransmitInit();
    #ifdef TEST
    slUART_WriteString("test Cmake.\r\n");
    #endif
    slUART_WriteString("Start.\r\n");
    // if (BME280_Init(BME280_OS_T_16, BME280_OS_P_16, BME280_OS_H_16, BME280_FILTER_16, BME280_MODE_NORMAL, BME280_TSB_62)) {
    if (BME280_Init(BME280_OS_T_1, BME280_OS_P_1, BME280_OS_H_1, BME280_FILTER_OFF, BME280_MODE_FORCED, BME280_TSB_1000)) {
        slUART_WriteString("BMP280 init error.\r\n");
    } else {
        slUART_WriteString("BMP280 init done.\r\n");
    }
    while (1) {
        LED_TOG;
        _delay_ms(100);
        if (BME280_SetMode(BME280_MODE_FORCED)){
            slUART_WriteString("Sensor set forced mode error!\r\n");
        } else {
            if (BME280_ReadAll(&temperature, &pressure, &humidity)) {
                slUART_WriteString("Sensor read error!\r\n");
            } else {
                sprintf(req, "Temp: %d.%02u Hum: %u.%02u Press: %u.%02u",
                        temperature / 100, temperature % 100,                            //C
                        //(pressure >> 8), ((pressure & 0x000000FF) * 100) >> 8,			//Pa
                        (pressure >> 8) / 100, (pressure >> 8) % 100,                    //hPa
                        humidity >> 10, ((humidity & 0x000003FF) * 100) >> 10);            //rH
                slUART_WriteString(req);
                slUART_WriteString("\r\n");
            }
            LED_TOG;
        }
        _delay_ms(5000);
    }
    return 0;
}