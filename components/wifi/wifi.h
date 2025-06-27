#pragma once

#define AP_WIFI_SSID CONFIG_ESP_AP_SSID
#define AP_WIFI_PASS CONFIG_ESP_AP_PASSWORD
#define AP_WIFI_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

void setup_wifi(void);
void ap_start(void);
void ap_stop(void);
