/*******************************************************************************
 * tcc_project
 * ESP-IDF SPI library for acceleration and temperature read from GY-91 board (BMP280 & MPU9250)
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/
#ifndef _BMP_MPU_h_
#define _BMP_MPU_h_

#include "stdint.h"

// PINs Config
// #define PWR  GPIO_NUM_19
#define SCLK GPIO_NUM_23    //SCL
#define MOSI GPIO_NUM_18    //SDA
#define MISO GPIO_NUM_5     //SDO
#define CS   GPIO_NUM_15    //ChipSelect MPU9250

#define CSB   GPIO_NUM_4    //ChipSelect BMP280

// Defaults
#define SPIBUS_READ     (0x80)  /*!< addr | SPIBUS_READ  */
#define SPIBUS_WRITE    (0x7F)  /*!< addr & SPIBUS_WRITE */

//MPU
#define MPU_CLOCK_PLL  3  //!< Selects automatically best pll source
//BME
#define TEMP_DATA_SIZE 3
#define CONFIG_BME280_REG_CTRL_MEAS 0b00111111    //The “ctrl_meas” register sets the data acquisition options of the device. 

//=============================MPU9250=====================================
//=========================================================================

typedef enum {
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_ALL,
}ACCEL_AXIS;
//=========================================================================
//REGISTERS
enum
{
    MPU9250_REG_USER_CTRL       = 0x6A,
    MPU9250_REG_PWR_MGMT_1      = 0x6B,
    MPU9250_REG_SMPLRT_DIV      = 0x19,
    MPU9250_REG_ACCEL_CONFIG    = 0x1c,

    MPU9250_REG_ACCEL_XOUT_H    = 0x3B,
    MPU9250_REG_ACCEL_XOUT_L    = 0x3C,
    MPU9250_REG_ACCEL_YOUT_H    = 0x3D,
    MPU9250_REG_ACCEL_YOUT_L    = 0x3E,
    MPU9250_REG_ACCEL_ZOUT_H    = 0x3F,
    MPU9250_REG_ACCEL_ZOUT_L    = 0x40,
}MPU9250_BYTE_REGISTER;

enum
{
    FIFO_EN         = 0b01000000,   //Bit 6
    I2C_MST_EN      = 0b00100000,   //Bit 5
    I2C_IF_DIS      = 0b00010000,   //Bit 4  – Disable I2C Slave module and put the serial interface in SPI mode only. 
    FIFO_RST        = 0b00000100,   //Bit 2
    I2C_MST_RST     = 0b00000010,   //Bit 1   
    SIG_COND_RST    = 0b00000001,   //Bit 0  - This bit also clears all the sensor registers.
}MPU9250_BIT_USER_CTRL;

enum
{
    PWR_1_H_RESET       = 0b10000000,   //Bit 7
    PWR_1_SLEEP         = 0b01000000,   //Bit 6
    PWR_1_CYCLE         = 0b00100000,   //Bit 5
    PWR_1_GYRO_STANDBY  = 0b00010000,   //Bit 4
    PWR_1_PD_PTAT       = 0b00001000,   //Bit 3
    PWR_1_CLK_INT       = 0b00000000,   //Bit [2:0] - Internal 20MHz oscillator
    PWR_1_CLK_PLL       = 0b00000001,   //Bit [2:0] - Auto selects the best available clock source 
    PWR_1_CLK_STOP      = 0b00000111,   //Bit [2:0] - Stops the clock and keeps timing generator in reset
}MPU9250_BIT_PWR_MGMT_1;

enum
{
    ax_st_en        = 0b10000000,   //Bit 7
    ay_st_en        = 0b01000000,   //Bit 6
    az_st_en        = 0b00100000,   //Bit 5
    ACCEL_FS_2G     = 0b00000000,   //Bit [3:4]
    ACCEL_FS_4G     = 0b00001000,   //Bit [3:4]
    ACCEL_FS_8G     = 0b00010000,   //Bit [3:4]
    ACCEL_FS_16G    = 0b00011000,   //Bit [3:4]
}MPU9250_BIT_ACCEL_CONFIG;
//=========================================================================

//=============================BME280=====================================
//=========================================================================
//REGISTERS
enum
{
    BMP280_REGISTER_DIG_T1              = 0x88,
    BMP280_REGISTER_DIG_T2              = 0x8A,
    BMP280_REGISTER_DIG_T3              = 0x8C,

    BMP280_REGISTER_DIG_P1              = 0x8E,
    BMP280_REGISTER_DIG_P2              = 0x90,
    BMP280_REGISTER_DIG_P3              = 0x92,
    BMP280_REGISTER_DIG_P4              = 0x94,
    BMP280_REGISTER_DIG_P5              = 0x96,
    BMP280_REGISTER_DIG_P6              = 0x98,
    BMP280_REGISTER_DIG_P7              = 0x9A,
    BMP280_REGISTER_DIG_P8              = 0x9C,
    BMP280_REGISTER_DIG_P9              = 0x9E,

    BMP280_REGISTER_CHIPID             = 0xD0,
    BMP280_REGISTER_VERSION            = 0xD1,
    BMP280_REGISTER_SOFTRESET          = 0xE0,

    BMP280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

    BMP280_REGISTER_CONTROL            = 0xF4,
    BMP280_REGISTER_CONFIG             = 0xF5,
    BMP280_REGISTER_PRESSUREDATA       = 0xF7,
    BMP280_REGISTER_TEMPDATA           = 0xFA,
};

/*=========================================================================*/

//=========================================================================
//CALIBRATION DATA
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bmp280_calib_data;
/*=========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t SensorInit(void);
// uint8_t BMP_Init(void);
float BMP_readTemperature(void);

// uint8_t MPU_Init(void);
uint8_t MPU_setSampleRate(uint16_t rate);
uint8_t MPU_readAcceleration(ACCEL_AXIS axis, float* accelOut);

#ifdef __cplusplus
}
#endif

#endif