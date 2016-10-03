/*
 * slI2C.h
 *
 *  Created on: 24-05-2016
 *      Author: db
 */

#ifndef SLI2C_H_
#define showDebugData 0
#define compileREadFunctions 1
#define SLI2C_H_
#define slI2C_Init_Called 0
#define slI2C_STARTError 1
#define slI2C_NoNACKError 3
#define slI2C_NoACKError 4
#define slI2C_NACK 0
#define slI2C_ACK 1
#define I2CBUSCLOCK 100000UL
uint8_t I2C_Error;
uint8_t slI2C_statusFlags;
void slI2C_SetSLA(uint16_t slaIn);
void slI2C_SetBusSpeed(uint16_t speed);
void slI2C_Init();
uint8_t slI2C_Start();
uint8_t slI2C_Stop();
uint8_t slI2C_WriteByte(uint8_t byte);
#if compileREadFunctions == 1
uint8_t slI2C_ReadByte_NACK();
uint8_t slI2C_ReadByte_ACK();
#endif
#endif /* SLI2C_H_ */
