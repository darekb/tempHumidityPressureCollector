#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "slI2C.h"
#include "slUart.h"
#include "bme280.h"

#define BME280_API
#ifdef BME280_API
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 I2C_routine(void);
#endif
void BME280_delay_msek(u32 msek);
s32 bme280_data_readout_template(void);
struct bme280_t bme280;
#ifdef BME280_API
#define SPI_READ  0x80
#define SPI_WRITE 0x7F
#define BME280_DATA_INDEX 1
#define BME280_ADDRESS_INDEX  2
/*--------------------------------------------------------------------------*
* The following function is used to map the I2C bus read, write, delay and
* device address with global structure bme280
*-------------------------------------------------------------------------*/
s8 I2C_routine(void) {
/*--------------------------------------------------------------------------*
 *  By using bme280 the following structure parameter can be accessed
 *  Bus write function pointer: BME280_WR_FUNC_PTR
 *  Bus read function pointer: BME280_RD_FUNC_PTR
 *  Delay function pointer: delay_msec
 *  I2C address: dev_addr
 *--------------------------------------------------------------------------*/
  bme280.bus_write = BME280_I2C_bus_write;
  bme280.bus_read = BME280_I2C_bus_read;
  bme280.dev_addr = BME280_I2C_ADDRESS2;
  bme280.delay_msec = BME280_delay_msek;
  slI2C_Start();
  slI2C_WriteByte(bme280.dev_addr  << 1);
  slI2C_Stop();
  return BME280_INIT_VALUE;
}


/************** I2C/SPI buffer length ******/
#define I2C_BUFFER_LEN 8

/*-------------------------------------------------------------------*
* This is a sample code for read and write the data by using I2C/SPI
* Use either I2C or SPI based on your need
* The device address defined in the bme280.h file
*-----------------------------------------------------------------------*/
 /* \Brief: The function is used as I2C bus write
 *  \Return : Status of the I2C write
 *  \param dev_addr : The device address of the sensor
 *  \param reg_addr : Address of the first register, will data is going to be written
 *  \param reg_data : It is a value hold in the array,
 *    will be used for write the value into the register
 *  \param cnt : The no of byte of data to be write
 */
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
  s32 iError = BME280_INIT_VALUE;
  u8 array[I2C_BUFFER_LEN];
  u8 stringpos = BME280_INIT_VALUE;
  array[BME280_INIT_VALUE] = reg_addr;
  for (stringpos = BME280_INIT_VALUE; stringpos < cnt; stringpos++) {
    array[stringpos + BME280_DATA_INDEX] = *(reg_data + stringpos);
  }
  /*
  * Please take the below function as your reference for
  * write the data using I2C communication
  * "IERROR = I2C_WRITE_STRING(DEV_ADDR, array, cnt+1)"
  * add your I2C write function here
  * iError is an return value of I2C read function
  * Please select your valid return value
  * In the driver SUCCESS defined as 0
    * and FAILURE defined as -1
  * Note :
  * This is a full duplex operation,
  * The first read data is discarded, for that extra write operation
  * have to be initiated. For that cnt+1 operation done in the I2C write string function
  * For more information please refer data sheet SPI communication:
  */
  return (s8)iError;
}

 /* \Brief: The function is used as I2C bus read
 *  \Return : Status of the I2C read
 *  \param dev_addr : The device address of the sensor
 *  \param reg_addr : Address of the first register, will data is going to be read
 *  \param reg_data : This data read from the sensor, which is hold in an array
 *  \param cnt : The no of data byte of to be read
 */
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
  s32 iError = BME280_INIT_VALUE;
  u8 array[I2C_BUFFER_LEN] = {BME280_INIT_VALUE};
  u8 stringpos = BME280_INIT_VALUE;
  array[BME280_INIT_VALUE] = reg_addr;
  /* Please take the below function as your reference
   * for read the data using I2C communication
   * add your I2C rad function here.
   * "IERROR = I2C_WRITE_READ_STRING(DEV_ADDR, ARRAY, ARRAY, 1, CNT)"
   * iError is an return value of SPI write function
   * Please select your valid return value
     * In the driver SUCCESS defined as 0
     * and FAILURE defined as -1
   */
  for (stringpos = BME280_INIT_VALUE; stringpos < cnt; stringpos++) {
    *(reg_data + stringpos) = array[stringpos];
  }
  return (s8)iError;
}



/*  Brief : The delay routine
 *  \param : delay in ms
*/
void BME280_delay_msek(u32 msek)
{
  /*Here you can write your own delay routine*/
}
#endif

//TODO add BME280 library
//TODO pomiary na rządanie
//TODO 
//TODO średnia wleczone dla pomiarów
//TODO implementacja RF module 433.92MHz

#define LED (1 << PB0)
#define LED_TOG PORTB ^= LED

int main(void) {
	slUART_SimpleTransmitInit();
	DDRB |= LED;
  bme280_data_readout_template();

	while (1) {

		LED_TOG;
		_delay_ms(5000);
	}
}



s32 bme280_data_readout_template(void)
{
  /* The variable used to assign the standby time*/
  u8 v_stand_by_time_u8 = BME280_INIT_VALUE;
  /* The variable used to read uncompensated temperature*/
  s32 v_data_uncomp_temp_s32 = BME280_INIT_VALUE;
  /* The variable used to read uncompensated pressure*/
  s32 v_data_uncomp_pres_s32 = BME280_INIT_VALUE;
  /* The variable used to read uncompensated pressure*/
  s32 v_data_uncomp_hum_s32 = BME280_INIT_VALUE;
  /* The variable used to read compensated temperature*/
  s32 v_comp_temp_s32[2] = {BME280_INIT_VALUE, BME280_INIT_VALUE};
  /* The variable used to read compensated pressure*/
  u32 v_comp_press_u32[2] = {BME280_INIT_VALUE, BME280_INIT_VALUE};
  /* The variable used to read compensated humidity*/
  u32 v_comp_humidity_u32[2] = {BME280_INIT_VALUE, BME280_INIT_VALUE};

  /* result of communication results*/
  s32 com_rslt = ERROR;
  #ifdef BME280_API
  I2C_routine();
  #endif
  com_rslt = bme280_init(&bme280);
  com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);
  com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
  com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_2X);
  com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_4X);
  com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
  com_rslt += bme280_get_standby_durn(&v_stand_by_time_u8);
  com_rslt += bme280_read_uncomp_temperature(&v_data_uncomp_temp_s32);
  com_rslt += bme280_read_uncomp_pressure(&v_data_uncomp_pres_s32);
  com_rslt += bme280_read_uncomp_humidity(&v_data_uncomp_hum_s32);
  com_rslt += bme280_read_uncomp_pressure_temperature_humidity(&v_data_uncomp_temp_s32, &v_data_uncomp_pres_s32, &v_data_uncomp_hum_s32);
  v_comp_temp_s32[0] = bme280_compensate_temperature_int32(v_data_uncomp_temp_s32);
  v_comp_press_u32[0] = bme280_compensate_pressure_int32(v_data_uncomp_pres_s32);
  v_comp_humidity_u32[0] = bme280_compensate_humidity_int32(v_data_uncomp_hum_s32);
  com_rslt += bme280_read_pressure_temperature_humidity(&v_comp_press_u32[1], &v_comp_temp_s32[1],  &v_comp_humidity_u32[1]);
  com_rslt += bme280_set_power_mode(BME280_SLEEP_MODE);
  return com_rslt;
}