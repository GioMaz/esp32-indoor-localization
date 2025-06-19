#pragma once

#define AP_WIFI_SSID "martin_router_king"
#define AP_WIFI_PASS "ciao123456"
#define AP_WIFI_MAX_STA_CONN 3

void setup_wifi(void);
void ap_start(void);
void ap_stop(void);
