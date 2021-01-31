/*******************************************************************************
 * tcc_project 
 * ESP-IDF Wifi API
 *
 * Copyright (c) 2021 Lucas Feitosa <https://github.com/LucasSFeitosa/tcc_project>\n
 *
 * MIT Licensed as described in the file LICENSE
 *******************************************************************************/
#ifndef _APP_WIFI_h_
#define _APP_WIFI_h_

void Wifi_Init(void);
bool Wifi_Start_STA(void);
bool Wifi_Start_AP(void);

#endif