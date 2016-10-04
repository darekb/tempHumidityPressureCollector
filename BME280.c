/*********************************************************************************
Copyright (c) 2016, Cosmin Plasoianu
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 ********************************************************************************/

#include <avr/io.h>
#include <string.h>
#include "slI2C.h"
#include "BME280.h"


#define ID_REG            0xD0
#define PRESS_MSB_REG    0xF7
#define CALIB_00_REG    0x88
#define CALIB_26_REG    0xE1
#define CONFIG_REG        0xF5
#define CTRL_MEAS_REG    0xF4
#define STATUS_REG        0xF3
#define CTRL_HUM_REG    0xF2


static struct {
    uint16_t dig_T1;
    uint16_t dig_T2;
    uint16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} CalibParam;

int32_t t_fine;        // t_fine carries fine temperature as global value


uint8_t I2C_WriteData(uint8_t device_addr, uint8_t register_addr, uint8_t *data, uint8_t length) {
    uint8_t cnt = 0;
    slI2C_Start();                    //Start
    device_addr &= 0xFE;//Device address
    slI2C_WriteByte(device_addr);
    slI2C_WriteByte(register_addr);
    while (cnt != length) {
        slI2C_WriteByte(data[cnt++]);
    }
    slI2C_Stop();                        //Stop
    return 0;
}

uint8_t I2C_ReadData(uint8_t device_addr, uint8_t register_addr, uint8_t *data, uint8_t length) {
    uint8_t cnt;
    slI2C_Start();
    device_addr &= 0xFE;
    slI2C_WriteByte(device_addr);
    slI2C_WriteByte(register_addr);
    slI2C_Start();
    device_addr |= 0x01;
    slI2C_WriteByte(device_addr);
    if (length) {
        if (length > 1) {
            for (cnt = 0; cnt < length - 1; cnt++) {
                data[cnt] = slI2C_ReadByte_ACK();
            }
        }
        data[length - 1] = slI2C_ReadByte_NACK();
        slI2C_Stop();                        //Stop
        return 0;
    }
}


/**********************************************************************
Return: 0 	  - Everything OK
		non 0 - Failed
Parameters:	os_t - Temperature Oversampling
			os_p - Pressure Oversampling
			os_h - Humidity Oversampling
			filter - Filter coefficient
			mode - Mode (Sleep/Forced/Normal)
			t_sb - Standby time between conversions
**********************************************************************/
uint8_t BME280_Init(uint8_t os_t, uint8_t os_p, uint8_t os_h,
                    uint8_t filter, uint8_t mode, uint8_t t_sb) {
    uint8_t ID = 0;
    uint8_t Buff[26] = {0};
    uint8_t Temp;

    I2C_ReadData(BME280_I2C_ADDR, ID_REG, &ID, 1);
    if (ID != 0x60)
        return 1;

    I2C_ReadData(BME280_I2C_ADDR, CALIB_00_REG, Buff, 26);

    //ToDo: test im_update bit
    CalibParam.dig_T1 = (Buff[1] << 8) | Buff[0];
    CalibParam.dig_T2 = (Buff[3] << 8) | Buff[2];
    CalibParam.dig_T3 = (Buff[5] << 8) | Buff[4];

    CalibParam.dig_P1 = (Buff[7] << 8) | Buff[6];
    CalibParam.dig_P2 = (Buff[9] << 8) | Buff[8];
    CalibParam.dig_P3 = (Buff[11] << 8) | Buff[10];
    CalibParam.dig_P4 = (Buff[13] << 8) | Buff[12];
    CalibParam.dig_P5 = (Buff[15] << 8) | Buff[14];
    CalibParam.dig_P6 = (Buff[17] << 8) | Buff[16];
    CalibParam.dig_P7 = (Buff[19] << 8) | Buff[18];
    CalibParam.dig_P8 = (Buff[21] << 8) | Buff[20];
    CalibParam.dig_P9 = (Buff[23] << 8) | Buff[22];

    CalibParam.dig_H1 = Buff[25];

    memset(Buff, 0, 7);
    I2C_ReadData(BME280_I2C_ADDR, CALIB_26_REG, Buff, 7);

    CalibParam.dig_H2 = (Buff[1] << 8) | Buff[0];
    CalibParam.dig_H3 = Buff[2];
    CalibParam.dig_H4 = (Buff[3] << 4) | (Buff[4] & 0x0F);
    CalibParam.dig_H5 = (Buff[5] << 4) | (Buff[4] >> 4);
    CalibParam.dig_H6 = Buff[6];

    Temp = (t_sb << 5) | ((filter & 0x07) << 2);                    //config
    I2C_WriteData(BME280_I2C_ADDR, CONFIG_REG, &Temp, 1);

    Temp = os_h & 0x07;                                                //hum
    I2C_WriteData(BME280_I2C_ADDR, CTRL_HUM_REG, &Temp, 1);

    Temp = (os_t << 5) | ((os_p & 0x07) << 2) | (mode & 0x03);        //meas
    I2C_WriteData(BME280_I2C_ADDR, CTRL_MEAS_REG, &Temp, 1);

    return 0;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of �5123� equals 51.23 DegC.
int32_t BME280_CompensateT(int32_t adc_T) {
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t) CalibParam.dig_T1 << 1))) * ((int32_t) CalibParam.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) CalibParam.dig_T1)) * ((adc_T >> 4) - ((int32_t) CalibParam.dig_T1))) >> 12) *
            ((int32_t) CalibParam.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of �24674867� represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t BME280_CompensateP(int32_t adc_P) {
    int64_t var1, var2, p;

    var1 = ((int64_t) t_fine) - 128000;
    var2 = var1 * var1 * (int64_t) CalibParam.dig_P6;
    var2 = var2 + ((var1 * (int64_t) CalibParam.dig_P5) << 17);
    var2 = var2 + (((int64_t) CalibParam.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t) CalibParam.dig_P3) >> 8) + ((var1 * (int64_t) CalibParam.dig_P2) << 12);
    var1 = (((((int64_t) 1) << 47) + var1)) * ((int64_t) CalibParam.dig_P1) >> 33;

    if (var1 == 0)
        return 0;    //avoid exception caused by division by zero

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t) CalibParam.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t) CalibParam.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t) CalibParam.dig_P7) << 4);

    return (uint32_t) p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of �47445� represents 47445/1024 = 46.333 %RH
uint32_t BME280_CompensateH(int32_t adc_H) {
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t) 76800));
    v_x1_u32r = (
            ((((adc_H << 14) - (((int32_t) CalibParam.dig_H4) << 20) - (((int32_t) CalibParam.dig_H5) * v_x1_u32r)) +
              ((int32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((int32_t) CalibParam.dig_H6)) >> 10) * (((v_x1_u32r *
                                                                                                        ((int32_t) CalibParam.dig_H3)) >>
                                                                                                       11) +
                                                                                                      ((int32_t) 32768))) >>
                                              10) + ((int32_t) 2097152)) *
                                            ((int32_t) CalibParam.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) CalibParam.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (uint32_t) (v_x1_u32r >> 12);
}

/**********************************************************************
Return: 0 	  - Everything OK
		non 0 - Failed
Parameters:	t - Pointer to variable in which to write the temperature
			p - Pointer to variable in which to write the pressure
			h - Pointer to variable in which to write the humidity
**********************************************************************/
uint8_t BME280_ReadAll(int32_t *t, uint32_t *p, uint32_t *h) {
    uint8_t Buff[8] = {0};
    int32_t UncT, UncP, UncH;

    if (I2C_ReadData(BME280_I2C_ADDR, PRESS_MSB_REG, Buff, 8))
        return 1;

    UncP = ((uint32_t) Buff[0] << 16) | ((uint16_t) Buff[1] << 8) | Buff[2];
    UncP >>= 4;

    UncT = ((uint32_t) Buff[3] << 16) | ((uint16_t) Buff[4] << 8) | Buff[5];
    UncT >>= 4;

    UncH = ((uint16_t) Buff[6] << 8) | Buff[7];

    *t = BME280_CompensateT(UncT);
    *p = BME280_CompensateP(UncP);
    *h = BME280_CompensateH(UncH);

    return 0;
}

/**********************************************************************
Return: 0 	  - Everything OK
		non 0 - Failed
Parameters:	mode - Mode (Sleep/Forced/Normal)
**********************************************************************/
uint8_t BME280_SetMode(uint8_t mode) {
    uint8_t RegVal = 0;

    mode &= 0x03;
    I2C_ReadData(BME280_I2C_ADDR, CTRL_MEAS_REG, &RegVal, 1);
    RegVal &= 0xFC;
    RegVal |= mode;
    return I2C_WriteData(BME280_I2C_ADDR, CTRL_MEAS_REG, &RegVal, 1);
}