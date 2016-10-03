/*
 * slUart.h
 *
 *  Created on: 12-04-2016
 *      Author: db
 */

#ifndef SLUART_H_
#define SLUART_H_

#define UART_BAUD 9600
#define __UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1)

void slUART_Init();
void slUART_SimpleTransmitInit();
void slUART_WriteString(const char myString[]);
void slUART_LogBinary(uint8_t dataIn);
void slUART_LogDec(uint8_t dataIn);
#endif /* SLUART_H_ */
