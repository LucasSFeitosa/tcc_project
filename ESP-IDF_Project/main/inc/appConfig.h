/*******************************************************************************
 * tcc_project
 * ESP-IDF NVS API
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>



#define CONFIG_MQTT_MAIN_TOPIC       "LSF/tcc/Automation/#"
#define CONFIG_MQTT_MAIN_TOPIC_LEN    (strlen(CONFIG_MQTT_MAIN_TOPIC) - 1)

#define CONFIG_AP_SSID          "Config Interface"
#define CONFIG_AP_PASSWORD      "12345678"

#define CONFIG_AP_CHANNEL           80
#define CONFIG_AP_AUTHMODE          WIFI_AUTH_WPA_WPA2_PSK
#define CONFIG_AP_MAX_CONNECTIONS   2
#define CONFIG_AP_BEACON_INTERVAL   100
#define CONFIG_AP_SSID_HIDDEN 0

#define LOGIN_MAX_LEN  32
#define PASSWD_MAX_LEN 24
#define ADDR_MAX_LEN 32

static const char* const NVS_FLASH_KEY_MQTT     = "mqtt";
static const char* const NVS_FLASH_KEY_WIFI_STA = "sta";

extern uint8_t OP_Mode;

enum {
    MODE_AUTO,
	MODE_MANUAL,
} Operation_Mode_et;

typedef enum {
    CMD_STAT = '0',
	CMD_RELAY_1,
	CMD_RELAY_2,
} CMD_Control_et;

typedef enum {
    CONN_STAT_DEINITIALIZED = 0,
	CONN_STAT_INITIALIZING,
	CONN_STAT_INITIALIZED,
	CONN_STAT_CONNECTING,
	CONN_STAT_CONNECTED,
} CONN_State_et;

typedef struct {
	char ssid  [LOGIN_MAX_LEN ];
    char passwd[PASSWD_MAX_LEN];
} CONFIG_Wifi_STA_st;

typedef struct {
	char ssid  [LOGIN_MAX_LEN ];
    char passwd[PASSWD_MAX_LEN];
} CONFIG_Wifi_AP_st;

typedef struct {
    char     address[ADDR_MAX_LEN];
    uint16_t port;
    char     login  [LOGIN_MAX_LEN ];
    char     passwd [PASSWD_MAX_LEN];
} CONFIG_Mqtt_st;

typedef struct {
	CONFIG_Wifi_STA_st STA;
	CONFIG_Wifi_AP_st AP;
} CONFIG_Interface_st;

typedef struct {
	CONFIG_Mqtt_st mqtt;
} CONFIG_Protocol_st;

typedef struct {
	CONFIG_Interface_st iface;
	CONFIG_Protocol_st  prot;
	CONN_State_et     	connState;
} CONFIG_Network_st;

#ifdef __cplusplus
extern "C"
{
#endif

void Conf_Init (void);
void Conf_Network_Load(void);
void Conf_Network_Save(void);
CONFIG_Network_st* Conf_GetNetworkInstance();

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H_ */