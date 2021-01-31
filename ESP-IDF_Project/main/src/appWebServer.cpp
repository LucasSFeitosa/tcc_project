/*******************************************************************************
 * tcc_project
 * ESP-IDF Web Server Project
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "sys/param.h"
#include "nvs_flash.h"
// #include "tcpip_adapter.h"
// #include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_http_server.h>

#include "appWebServer.h"
#include "bmp_mpu.h"
#include "appIO.h"

#define EXAMPLE_ESP_WIFI_SSID      "mywifissid"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "webServer";
httpd_handle_t server = NULL;

/* An HTTP GET handler */
esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    // httpd_resp_send(req, "Click to go to the next page <a href=\"/main\">here</a> <br>", -1); // -1 = use strlen()
    extern const unsigned char index_start[] asm("_binary_index_html_start");
    extern const unsigned char index_end[]   asm("_binary_index_html_end");
    const size_t index_size = (index_end - index_start);

    /* Add file upload form and script which on execution sends a POST request to /upload */
    httpd_resp_send_chunk(req, (const char *)index_start, index_size);
    // httpd_resp_set_type(req, "text/html");
    // httpd_resp_send(req, "<h1>Hello Secure World!</h1>", -1); // -1 = use strlen()
    return ESP_OK;
}

esp_err_t main_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "main handler");
    // // httpd_resp_set_type(req, "text/html");
    // // httpd_resp_send(req, "<h1>Hello Secure World!</h1>", -1); // -1 = use strlen()
    return ESP_OK;
}

esp_err_t temp_get_handler(httpd_req_t *req)
{
    char temp_str[10];
    float temp = BMP_readTemperature();
    httpd_resp_set_type(req, "text/plain"); 
    ESP_LOGI(TAG, "temp: [%f] *C \r\n", temp);

    sprintf(temp_str, "%.2f", temp);

    httpd_resp_sendstr(req, temp_str); 
    return ESP_OK;
}

esp_err_t accel_get_handler(httpd_req_t *req)
{
    char accel_str[10];
    float acc[3] = {0};
    httpd_resp_set_type(req, "text/plain");
    MPU_readAcceleration(AXIS_ALL, acc);  
    ESP_LOGI(TAG, "accel: [%f] (G) \r\n", acc[0]);

    sprintf(accel_str, "%f,%f,%f", acc[0], acc[1], acc[2]);

    httpd_resp_sendstr(req, accel_str); 
    return ESP_OK;
}

esp_err_t motorStat_get_handler(httpd_req_t *req)
{
    char send_str[10];
    // float acc;
    httpd_resp_set_type(req, "text/plain");
    
    // ESP_LOGI(TAG, "accel: [%d] (G) \r\n", !IO_getStateRelay());

    sprintf(send_str, "%d", !IO_getStateRelay());

    httpd_resp_sendstr(req, send_str); 
    return ESP_OK;
}

esp_err_t toogleRelay_get_handler(httpd_req_t *req)
{
    char send_str[10];
    // float acc;
    httpd_resp_set_type(req, "text/plain");
    
    // ESP_LOGI(TAG, "accel: [%d] (G) \r\n", !IO_getStateRelay());
    if (OP_Mode == MODE_MANUAL) 
        sprintf(send_str, "%d", !IO_relayToggle());
    else 
        sprintf(send_str, "%d", !IO_getStateRelay());

    httpd_resp_sendstr(req, send_str); 
    return ESP_OK;
}

esp_err_t config_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    // httpd_resp_send(req, "Click to go to the next page <a href=\"/main\">here</a> <br>", -1); // -1 = use strlen()
    extern const unsigned char index_start[] asm("_binary_configPage_html_start");
    extern const unsigned char index_end[]   asm("_binary_configPage_html_end");
    const size_t index_size = (index_end - index_start);

    httpd_resp_send_chunk(req, (const char *)index_start, index_size);

    return ESP_OK;
}

esp_err_t saveConfig_post_handler(httpd_req_t *req)  
{
    char* buf;
    int ret, remaining = req->content_len;
    CONFIG_Network_st* Conf = Conf_GetNetworkInstance();

    buf = (char*)calloc(remaining, sizeof(char));
    int size = remaining;
    
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, remaining))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        
        remaining -= ret;
    }
    ESP_LOGI(TAG, "Previous OP_Mode %d", OP_Mode);
    ESP_LOGI(TAG, "buf: %s", buf);

    char bufQuery[50];
    if (httpd_query_key_value(buf, "opMode", bufQuery, 50) == ESP_OK)
    {
        ESP_LOGI(TAG, "bufQuery: %s", bufQuery);
        if (strstr(bufQuery,"Manual") != NULL) 
        {
            ESP_LOGW(TAG, "MODE_MANUAL");
            OP_Mode = MODE_MANUAL; 
        }
        else
        {
            ESP_LOGW(TAG, "MODE_AUTO");
            OP_Mode = MODE_AUTO; 
        }
    }
    else if (httpd_query_key_value(buf, "wifi-ssid", bufQuery, 50) == ESP_OK)
    {
        httpd_query_key_value(buf, "wifi-ssid", Conf->iface.STA.ssid, LOGIN_MAX_LEN);
        httpd_query_key_value(buf, "wifi-password", Conf->iface.STA.passwd, PASSWD_MAX_LEN);
        Conf_Network_Save();
    }

    free(buf);

    // End response
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/config");
    httpd_resp_sendstr(req, "Changes Saved");
    return ESP_OK;
}

esp_err_t echo_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "ECHO handler");
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

const httpd_uri_t page_handlers[] = {
   {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_get_handler
    },
    {
        .uri       = "/main",
        .method    = HTTP_GET,
        .handler   = main_get_handler,
        .user_ctx = NULL,
    },
    {
        .uri       = "/temp",
        .method    = HTTP_GET,
        .handler   = temp_get_handler,
        .user_ctx = NULL,
    },
    {
        .uri       = "/accel",
        .method    = HTTP_GET,
        .handler   = accel_get_handler,
        .user_ctx = NULL,
    },
    {
        .uri       = "/motorStat",
        .method    = HTTP_GET,
        .handler   = motorStat_get_handler,
        .user_ctx = NULL,
    },
    {
        .uri       = "/toogleRelay",
        .method    = HTTP_GET,
        .handler   = toogleRelay_get_handler,
        .user_ctx = NULL,
    },
    {
        .uri       = "/config",
        .method    = HTTP_GET,
        .handler   = config_get_handler,
        .user_ctx = NULL,
    },
    {
        .uri       = "/saveConfig",
        .method    = HTTP_POST,
        .handler   = saveConfig_post_handler,
        .user_ctx  = NULL,
    },
    {
        .uri       = "/echo",
        .method    = HTTP_POST,
        .handler   = echo_post_handler,
        .user_ctx  = NULL,
    }
};

int page_handlers_no = sizeof(page_handlers)/sizeof(httpd_uri_t);
void register_page_handlers(httpd_handle_t hd)
{
    int i;
    ESP_LOGI(TAG, "Registering basic handlers");
    ESP_LOGI(TAG, "No of handlers = %d", page_handlers_no);
    for (i = 0; i < page_handlers_no; i++) {
        if (httpd_register_uri_handler(hd, &page_handlers[i]) != ESP_OK) {
            ESP_LOGW(TAG, "register uri failed for %d", i);
            return;
        }
    }
    ESP_LOGI(TAG, "Success");
}

bool start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        register_page_handlers(server);
    
        return 1;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return 0;
}

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}
