#include "appWifi.h"

#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"

// #include "lwip/sockets.h"
// #include "lwip/dns.h"
// #include "lwip/netdb.h"
#include "tcpip_adapter.h"
// #include "mqtt_client.h"

#include "appConfig.h"
#include "appMqtt.h"
#include "appWebServer.h"

static const char *TAG = "appWifi";

CONFIG_Interface_st* ifaceConfig;

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event);

void Wifi_Init(void)
{
    ifaceConfig = &Conf_GetNetworkInstance()->iface;
    // initialize the tcp stack
    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // initialize the wifi event handler
    ESP_ERROR_CHECK(esp_event_loop_init(Wifi_EventHandler, NULL)); 
}

// bool Wifi_Deinit(void)
// {

// }

bool Wifi_Start_STA(void)
{
    ESP_LOGW(TAG, "Wifi_Start_STA\n");
    uint8_t res;
    wifi_config_t   wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifi_config_t));

    if( ifaceConfig->STA.ssid[0] == '\0')
    {
        ESP_LOGI(TAG, "No SSid saved");
        return false;
    }
    strcpy((char *)wifiConfig.sta.ssid     , ifaceConfig->STA.ssid);
    strcpy((char *)wifiConfig.sta.password , ifaceConfig->STA.passwd);

    esp_wifi_stop();

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig));

    // start the wifi interface
    res = esp_wifi_start();

    ESP_LOGW(TAG, "Starting wifi STA\n");

    return res == ESP_OK;
}

bool Wifi_Start_AP(void)
{
    uint8_t res;
    wifi_config_t   wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifi_config_t));

    esp_wifi_stop();

    // stop DHCP server
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    
    // assign a static IP to the network interface
    tcpip_adapter_ip_info_t info;
    memset(&info, 0, sizeof(info));
    IP4_ADDR(&info.ip, 192, 168, 1, 1);
    IP4_ADDR(&info.gw, 192, 168, 1, 1);
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
    
    // start the DHCP server   
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

    strcpy((char *)wifiConfig.ap.ssid , CONFIG_AP_SSID);
    strcpy((char *)wifiConfig.ap.password , CONFIG_AP_PASSWORD);

    wifiConfig.ap.ssid_len        = 0;
    wifiConfig.ap.channel         = CONFIG_AP_CHANNEL;
    wifiConfig.ap.authmode        = CONFIG_AP_AUTHMODE;
    wifiConfig.ap.ssid_hidden     = CONFIG_AP_SSID_HIDDEN;
    wifiConfig.ap.max_connection  = CONFIG_AP_MAX_CONNECTIONS;
    wifiConfig.ap.beacon_interval = CONFIG_AP_BEACON_INTERVAL;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfig));

    // start the wifi interface
    res = esp_wifi_start();

    ESP_LOGI(TAG, "Starting wifi STA\n");
    
    return res == ESP_OK;
}

esp_err_t Wifi_EventHandler(void *ctx, system_event_t *event)
{
    static uint8_t numTry = 0;
    // httpd_handle_t *server = (httpd_handle_t *) ctx;
    //tcpip_adapter_ip_info_t ip;
    switch (event->event_id) {

        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "LSF-Automation")); 
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            // Mqtt_Init();
            start_webserver();
            numTry = 0;
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            // Mqtt_Deinit();
            stop_webserver();
            if (numTry >= 5)
            {   //Numero maximo de tentativas atingidas
                // esp_restart();
                Wifi_Start_AP();
            }
            else
            {
                numTry++;
            }
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_AP_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_START"); 
            break;
                
        case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STACONNECTED");
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STADISCONNECTED");
            break;

        default:
            break;
    }
    return ESP_OK;
}