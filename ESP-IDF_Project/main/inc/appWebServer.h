#ifndef APP_WEB_SERVER_H_
#define APP_WEB_SERVER_H_

#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>

#include <esp_http_server.h>
#include <esp_http_client.h>

#include "appConfig.h"

bool start_webserver(void);
void stop_webserver(void);

#endif
