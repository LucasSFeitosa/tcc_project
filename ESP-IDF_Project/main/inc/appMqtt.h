/*******************************************************************************
 * tcc_project
 * ESP-IDF MQTT API
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/

#ifndef _APP_Mqtt_h_
#define _APP_Mqtt_h_

#include "mqtt_client.h"

bool    Mqtt_Init(void);
void    Mqtt_Deinit(void);
uint8_t Mqtt_Disconnect(void);
bool    Mqtt_SendMesage(const char *topic, const char *data, int len, int qos, int retain);
bool    Mqtt_Subscribe(const char *topic);
void    Mqtt_DataCallback_SET(mqtt_event_callback_t eventDataCallBack_p);

#endif