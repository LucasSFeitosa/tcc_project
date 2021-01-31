/*******************************************************************************
 * tcc_project
 * ESP-IDF NVS API
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/
#include "esp_system.h"
#include "esp_log.h"
#include "esp_flash_encrypt.h"
#include "sys/param.h"
#include "nvs_flash.h"
#include <string.h>

#include "appConfig.h"

static const char* TAG = "appConfig";

static const char* const NVS_FLASH_CONFIG_PARTITION = "config";

CONFIG_Network_st networkConfig;
uint8_t OP_Mode = MODE_AUTO;

uint8_t writeNVS(const char* key, void* data, uint32_t dataSize);
uint8_t readNVS(const char* key, void* dataOut, uint32_t dataSize);

void Conf_Init (void)
{
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
    memset(&networkConfig, 0, sizeof(networkConfig));

    Conf_Network_Load();
}

void Conf_Network_Load(void)
{
    strcpy(networkConfig.iface.AP.ssid, CONFIG_AP_SSID);
    strcpy(networkConfig.iface.AP.passwd, CONFIG_AP_PASSWORD);

    readNVS(NVS_FLASH_KEY_WIFI_STA, &networkConfig.iface.STA, sizeof(CONFIG_Wifi_STA_st));
    readNVS(NVS_FLASH_KEY_MQTT, &networkConfig.prot.mqtt, sizeof(CONFIG_Mqtt_st));
}

void Conf_Network_Save(void)
{
    writeNVS(NVS_FLASH_KEY_WIFI_STA, &networkConfig.iface.STA, sizeof(CONFIG_Wifi_STA_st));
    writeNVS(NVS_FLASH_KEY_MQTT, &networkConfig.prot.mqtt, sizeof(CONFIG_Mqtt_st));
}

CONFIG_Network_st* Conf_GetNetworkInstance()
{
    return &networkConfig;
} 

uint8_t writeNVS(const char* key, void* data, uint32_t dataSize)
{   
    uint8_t result = 0;
    nvs_handle handle = 0;
	esp_err_t err = nvs_open(NVS_FLASH_CONFIG_PARTITION, NVS_READWRITE, &handle);

	if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "FAIL Open NVS Partition");
        return result;
    } 

	err = nvs_set_blob(handle, key, data, dataSize);

	if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "FAIL Set blob");
        return result;
    } 

	err = nvs_commit(handle);
	if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "FAIL commit blob");
        return result;
    } 

	nvs_close(handle);
    result = 1;
    return result;
}

uint8_t readNVS(const char* key, void* dataOut, uint32_t dataSize)
{
    uint8_t    result = 0;
    nvs_handle handle = 0;
    esp_err_t  err    = nvs_open(NVS_FLASH_CONFIG_PARTITION, NVS_READONLY, &handle);
    uint32_t   requiredSize;
   
    if (err  != ESP_OK)
    {
        ESP_LOGI(TAG, "FAIL Open NVS Partition");
        memset(dataOut, 0, dataSize);
        return result;
    }

	err = nvs_get_blob(handle, key, NULL, &requiredSize);

	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "FAIL Get Blob");
        nvs_close(handle);
        memset(dataOut, 0, dataSize);
        return result;
    }

	if (requiredSize == 0)
    {
		 ESP_LOGI(TAG, "Blob not saved yet");
	} 
    else 
    {
		err = nvs_get_blob(handle, key, dataOut, &dataSize);

		if (err != ESP_OK)
        {
            nvs_close(handle);
            memset(dataOut, 0, dataSize);
            return result;
        } 
	}

	nvs_close(handle);
    result = 1;

    return result;
}