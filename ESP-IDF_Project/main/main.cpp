#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
// #include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#include "string.h"

#include "appConfig.h"
#include "appWifi.h"
#include "appMqtt.h"
#include "bmp_mpu.h"
#include "appIO.h"

static const char* TAG = "main";

TaskHandle_t mainTaskHandle;

void mainTask (void* pvParameter);
void Set_HardCodeConfig (void);

extern "C" void app_main() {
    IO_peripheralsInit();
    Conf_Init();
    // Set_HardCodeConfig();

    if(xTaskCreate(mainTask, "MainTask", (1024*5), NULL, configMAX_PRIORITIES, &mainTaskHandle) != pdPASS )
    {
        ESP_LOGW(TAG, "Fail to create MainTask");
    }
}

esp_err_t DataRX_Callback(esp_mqtt_event_handle_t event)
{
    char subTopic[30];
    
    strncpy(subTopic, event->topic + CONFIG_MQTT_MAIN_TOPIC_LEN, (event->topic_len - CONFIG_MQTT_MAIN_TOPIC_LEN));
    subTopic[event->topic_len - CONFIG_MQTT_MAIN_TOPIC_LEN] = '\0';
    ESP_LOGI(TAG, "%c", event->topic[CONFIG_MQTT_MAIN_TOPIC_LEN]);
    ESP_LOGI(TAG, "%s", subTopic);

    switch ( event->topic[CONFIG_MQTT_MAIN_TOPIC_LEN])
    {
        case CMD_STAT:
            /* code */
            break;
        case CMD_RELAY_1:

            break;
        case CMD_RELAY_2:
            break;
        default:
            break;
    }

    return ESP_OK;
}

void mainTask (void* pvParameter)
{
    SensorInit();
    BMP_readTemperature();//discart first meassurement
    
    Wifi_Init();
    Wifi_Start_STA();
    // Mqtt_DataCallback_SET(DataRX_Callback);
    
    float temp;
    float acc[3];
    while (1)
    {
        if (OP_Mode == MODE_AUTO)
        {
            ESP_LOGI(TAG, "LevelSensor %d - IO_getStateRelay %d", IO_getStateLevelSensor(), IO_getStateRelay());
            if(IO_getStateLevelSensor() != IO_getStateRelay())
            {
                IO_setStateRelay(IO_getStateLevelSensor());
            }
        }
        // ESP_LOGI(TAG, "Temperature: %f *C", BMP_readTemperature());
        // MPU_readAcceleration(AXIS_ALL, acc);   
        // printf("accel: [%f; %f; %f] (G) \r\n", acc[0], acc[1], acc[2]);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
    
}

void Set_HardCodeConfig (void)
{
    CONFIG_Network_st* Conf = Conf_GetNetworkInstance();
    strcpy(Conf->iface.STA.ssid, "Lucas_2");
    strcpy(Conf->iface.STA.passwd, "123654");

    strcpy(Conf->prot.mqtt.address, "broker.hivemq.com");
    Conf->prot.mqtt.port = 1883;

    Conf_Network_Save();
}