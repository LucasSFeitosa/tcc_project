#include "bmp_mpu.h"

#include <stdint.h>
#include <string.h>
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"

#define SPI_CLOCK_SPEED_Hz 1000000

void BMP_readCoefficients(void);

spi_device_handle_t mpu_spi_handle;

spi_device_handle_t bmp_spi_handle;
bmp280_calib_data   bmp_calib;

uint8_t accelScale = 2;

esp_err_t SPIreadBytes(spi_device_handle_t handle, uint8_t regAddr, size_t length, uint8_t *data) {
    if(length == 0) return ESP_ERR_INVALID_SIZE;
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = regAddr | SPIBUS_READ;
    transaction.length = length * 8;
    transaction.rxlength = length * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = data;
    esp_err_t err = spi_device_transmit(handle, &transaction);    
    return err;
}

esp_err_t SPIwriteBytes(spi_device_handle_t handle, uint8_t regAddr, size_t length, const uint8_t *data) {
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = regAddr & SPIBUS_WRITE;
    transaction.length = length * 8;
    transaction.rxlength = 0;
    transaction.user = NULL;
    transaction.tx_buffer = data;
    transaction.rx_buffer = NULL;
    esp_err_t err = spi_device_transmit(handle, &transaction);

    return err;
}

// esp_err_t read16

uint8_t BMP_Init(void)
{
    uint8_t bmp280_chip_ID;

    spi_device_interface_config_t dev_config;
    dev_config.command_bits = 0;
    dev_config.address_bits = 8;
    dev_config.dummy_bits = 0;
    dev_config.mode = 0;
    dev_config.duty_cycle_pos = 128;
    dev_config.cs_ena_pretrans = 0; 
    dev_config.cs_ena_posttrans = 0;
    dev_config.clock_speed_hz = SPI_CLOCK_SPEED_Hz;
    dev_config.spics_io_num = CSB;
    dev_config.flags = SPI_DEVICE_HALFDUPLEX;
    dev_config.queue_size = 1;
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &bmp_spi_handle));

    uint8_t cmd = 0xB6;
    SPIreadBytes(bmp_spi_handle, 0xE0, sizeof(uint8_t), (uint8_t*)&cmd); //reset
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_CHIPID, sizeof(uint8_t), (uint8_t*)&bmp280_chip_ID);
    if (bmp280_chip_ID != 0x58)
        return false;

    BMP_readCoefficients();
    
    uint8_t ctrl_meas = CONFIG_BME280_REG_CTRL_MEAS;
    SPIwriteBytes(bmp_spi_handle, BMP280_REGISTER_CONTROL, sizeof(uint8_t), (uint8_t*)&ctrl_meas);
    return true;
}

void BMP_readCoefficients(void)
{
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_T1, sizeof(uint16_t), (uint8_t*)&bmp_calib.dig_T1);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_T2, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_T2);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_T3, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_T3);

    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P1, sizeof(uint16_t), (uint8_t*)&bmp_calib.dig_P1);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P2, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P2);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P3, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P3);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P4, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P4);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P5, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P5);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P6, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P6);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P7, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P7);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P8, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P8);
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_DIG_P9, sizeof(int16_t), (uint8_t*)&bmp_calib.dig_P9);
}

float BMP_readTemperature(void)
{
    int32_t var1, var2, T;
    uint8_t buffer[4];
    int32_t adc_T;
    // uint8_t *x = &adc_T;
    SPIreadBytes(bmp_spi_handle, BMP280_REGISTER_TEMPDATA, TEMP_DATA_SIZE, buffer);
    adc_T = buffer[0] << 12 | buffer[1] << 4 | buffer[2] >> 4;
    // printf("\n raw temperature %i\n", adc_T);
    // printf("\n raw temperature1 %02x %02x %02x %02x \n", x[0], x[1], x[2], x[3]);
    // adc_T >>= 4;
    // printf("\n raw temperature2 %02x %02x %02x %02x \n", x[0], x[1], x[2], x[3]);
    var1  = ((((adc_T>>3) - ((int32_t)bmp_calib.dig_T1 <<1))) * ((int32_t)bmp_calib.dig_T2)) >> 11;

    var2  = (((((adc_T>>4) - ((int32_t)bmp_calib.dig_T1)) * ((adc_T>>4) - ((int32_t)bmp_calib.dig_T1))) >> 12) *
        ((int32_t)bmp_calib.dig_T3)) >> 14;

    int32_t t_fine = var1 + var2;

    // printf ("t_fine %i\n", t_fine);

    T  = (t_fine * 5 + 128) >> 8;
    return (float)T/100.0;
    // double var1, var2, T;
    // var1 = (((double)adc_T)/16384.0 - ((double)bmp_calib.dig_T1)/1024.0) * ((double)bmp_calib.dig_T2);
    // var2 = ((((double)adc_T)/131072.0 - ((double)bmp_calib.dig_T1)/8192.0) *
    // (((double)adc_T)/131072.0 - ((double) bmp_calib.dig_T1)/8192.0)) * ((double)bmp_calib.dig_T3);
   
    // T = (var1 + var2) / 5120.0;
    // return T;
}

uint8_t MPU_Init(void)
{
    spi_device_interface_config_t dev_config;
    dev_config.command_bits = 0;
    dev_config.address_bits = 8;
    dev_config.dummy_bits = 0;
    dev_config.mode = 0;
    dev_config.duty_cycle_pos = 128;
    dev_config.cs_ena_pretrans = 0; 
    dev_config.cs_ena_posttrans = 0;
    dev_config.clock_speed_hz = SPI_CLOCK_SPEED_Hz;
    dev_config.spics_io_num = CS;
    dev_config.flags = SPI_DEVICE_HALFDUPLEX;
    dev_config.queue_size = 1;
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &mpu_spi_handle));

    uint8_t cmd;

    cmd = PWR_1_H_RESET;    //Reset Config
    SPIwriteBytes(mpu_spi_handle, MPU9250_REG_PWR_MGMT_1, sizeof(uint8_t), (uint8_t*)&cmd);

    cmd = PWR_1_CLK_PLL;    //Set Clock Source
    SPIwriteBytes(mpu_spi_handle, MPU9250_REG_PWR_MGMT_1, sizeof(uint8_t), (uint8_t*)&cmd);

    cmd = I2C_IF_DIS;    //Disable I2C
    SPIwriteBytes(mpu_spi_handle, MPU9250_REG_USER_CTRL, sizeof(uint8_t), (uint8_t*)&cmd);

    // cmd = ax_st_en | ay_st_en | az_st_en | ACCEL_FS_8G;
    //Config Accel Sensor
    switch (accelScale)
    {
        case 16:
            cmd = ACCEL_FS_16G;
            break;
        case 8:
            cmd = ACCEL_FS_8G;
            break;
        case 4:
            cmd = ACCEL_FS_4G;
            break;
        case 2:
        default:
            cmd = ACCEL_FS_2G;
            break;
    } 
    SPIwriteBytes(mpu_spi_handle, MPU9250_REG_ACCEL_CONFIG, sizeof(uint8_t), (uint8_t*)&cmd);

    MPU_setSampleRate(1000);
    return true;
}

uint8_t MPU_setSampleRate(uint16_t rate)
{
    esp_err_t err = ESP_FAIL;
    if (rate < 4) {
        rate = 4;
    }
    else if (rate > 1000) {
        rate = 1000;
    }
    uint16_t defaultSampleRate  = 1000;
    uint8_t divider            = defaultSampleRate / rate - 1;
    
    // ESP_LOGI(TAG,"Sample rate set to %d Hz", (defaultSampleRate / (1 + divider)));

    SPIwriteBytes(mpu_spi_handle, MPU9250_REG_SMPLRT_DIV, sizeof(uint8_t), &divider);
    return err == ESP_OK;
}

uint8_t MPU_readAcceleration(ACCEL_AXIS axis, float* accelOut)
{
    esp_err_t err = ESP_FAIL;
    uint8_t buffer[6];
    int16_t raw_data[3];
    switch (axis)
    {
        case AXIS_X:
        {
            err = SPIreadBytes(mpu_spi_handle, MPU9250_REG_ACCEL_XOUT_H, sizeof(int16_t), buffer);
            raw_data[0] = buffer[0] << 8 | buffer[1];
            accelOut[0] = (float)raw_data[0] * ((float)accelScale/INT16_MAX);
            break;
        }
        case AXIS_Y:
            err = SPIreadBytes(mpu_spi_handle, MPU9250_REG_ACCEL_YOUT_H, sizeof(int16_t), buffer);
            raw_data[0] = buffer[0] << 8 | buffer[1];
            accelOut[0] = (float)raw_data[0] * ((float)accelScale/INT16_MAX);
            break;
        case AXIS_Z:
            err = SPIreadBytes(mpu_spi_handle, MPU9250_REG_ACCEL_ZOUT_H, sizeof(int16_t), buffer);
            raw_data[0] = buffer[0] << 8 | buffer[1];
            accelOut[0] = (float)raw_data[0] * ((float)accelScale/INT16_MAX);
            break;
        case AXIS_ALL:
            err = SPIreadBytes(mpu_spi_handle, MPU9250_REG_ACCEL_XOUT_H, sizeof(int16_t) * 3, buffer);
            raw_data[0] = buffer[0] << 8 | buffer[1];
            raw_data[1] = buffer[2] << 8 | buffer[3];
            raw_data[2] = buffer[4] << 8 | buffer[5];
            accelOut[0] = (float)raw_data[0] * ((float)accelScale/INT16_MAX);
            accelOut[1] = (float)raw_data[1] * ((float)accelScale/INT16_MAX);
            accelOut[2] = (float)raw_data[2] * ((float)accelScale/INT16_MAX);
            break;
        default:
            break;
    }

    return err == ESP_OK;
}

uint8_t SensorInit(void)
{
    spi_bus_config_t spi_config;
    spi_config.mosi_io_num = MOSI;
    spi_config.miso_io_num = MISO;
    spi_config.sclk_io_num = SCLK;
    spi_config.quadwp_io_num = -1;
    spi_config.quadhd_io_num = -1;
    spi_config.max_transfer_sz = SPI_MAX_DMA_LEN;
    spi_config.flags           = 0;
    spi_config.intr_flags      = ESP_INTR_FLAG_LEVEL1;
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &spi_config, 0));

    return MPU_Init() && BMP_Init();
}

