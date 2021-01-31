/*******************************************************************************
 * tcc_project
 * ESP-IDF MQTT API
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "appConfig.h"
#include "appMqtt.h"

#define MAX_NUM_TRY 8

static const char *TAG = "LI_MQTT_WIFI_ETH";

esp_mqtt_client_handle_t mqttClient;
CONFIG_Network_st* networkHandle;

mqtt_event_callback_t eventDataCallBack = NULL;

esp_err_t Mqtt_EventHandler(esp_mqtt_event_handle_t event);

bool Mqtt_Init(void)
{
    char                     uriBuffer[50] = {0};
    esp_err_t                res           = ESP_FAIL;
    esp_mqtt_client_config_t mqttClientConfig;
    ESP_LOGI(TAG, "[IN ] MQTT_Init");

    networkHandle = Conf_GetNetworkInstance();

    if(networkHandle->prot.mqtt.address[0] == '\0' || networkHandle->prot.mqtt.port == 0)
        return 0;

    memset(&mqttClientConfig, 0, sizeof(esp_mqtt_client_config_t));   //It is necessary to set initial values to 0

    mqttClientConfig.event_handle = Mqtt_EventHandler;

    sprintf(uriBuffer, "mqtt://%s:%d",  networkHandle->prot.mqtt.address, networkHandle->prot.mqtt.port);

    //  sprintf(uriBuffer, "mqtt://http://test.mosquitto.org:1883", (char *)s_networkHandle->prot.mqtt.address, s_networkHandle->prot.mqtt.port);
    //  strcpy(uriBuffer, "mqtt://broker.hivemq.com:1883");
    mqttClientConfig.uri = (const char*)uriBuffer;

    mqttClient = esp_mqtt_client_init(&mqttClientConfig);
    res        = esp_mqtt_client_start(mqttClient);

    ESP_LOGI(TAG, "[OUT] MQTT_Init - %d", (uint8_t)(res == ESP_OK) );

    return res == ESP_OK; 
}

void Mqtt_Deinit(void)
{
    esp_mqtt_client_destroy(mqttClient);
}

void Mqtt_Reconnect(void)
{
    esp_mqtt_client_reconnect(mqttClient);
}

bool Mqtt_SendMesage(const char *topic, const char *data, int len, int qos, int retain)
{   
    return esp_mqtt_client_publish(mqttClient, topic, data, len, qos, retain) != -1 ? true : false;
}

bool Mqtt_Subscribe(const char *topic)
{   
    return esp_mqtt_client_subscribe(mqttClient, topic, 1) != -1 ? true : false;
}

void Mqtt_DataCallback_SET(mqtt_event_callback_t eventDataCallBack_p)
{
    eventDataCallBack = eventDataCallBack_p;
}

esp_err_t Mqtt_EventHandler(esp_mqtt_event_handle_t event)
{
    static uint8_t numTry = 0;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:  {
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            if (!Mqtt_Subscribe(CONFIG_MQTT_MAIN_TOPIC))
            {
                Mqtt_Reconnect();
            }        
            break;
        }
        case MQTT_EVENT_DISCONNECTED:   { 
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        }
        case MQTT_EVENT_SUBSCRIBED: {
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
            networkHandle->connState = CONN_STAT_CONNECTED;
            numTry = 0;
            break;
        }
        case MQTT_EVENT_UNSUBSCRIBED:   {
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
            Mqtt_Reconnect();
            break;
        }
        case MQTT_EVENT_PUBLISHED:  {
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        }
        case MQTT_EVENT_DATA:	{
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            if (event->data_len > 0)
            {
                ESP_LOGI(TAG, "WIFIETH RX -> : %d", event->data_len);
                // for (uint16_t pos = 0; pos < event->data_len; pos++)
                // {
                //     printf("%02x ", event->data[pos]);
                // }            
                // printf("\n");
                if(eventDataCallBack)
                    eventDataCallBack(event);
            } 
            break;
        }
        case MQTT_EVENT_BEFORE_CONNECT: { //Start connect attempt
            if (numTry >= MAX_NUM_TRY)
            {   //Numero maximo de tentativas atingidas
                esp_restart();
            }
            else
            {
                numTry++;
            }
            break;
        }
        case MQTT_EVENT_ERROR:  {
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        }
        default:    {
            ESP_LOGI(TAG, "Other event -  id:%d", event->event_id);
            break;
        }
    }
    return ESP_OK;
}