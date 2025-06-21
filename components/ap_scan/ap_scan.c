#include "ap_scan.h"
#include "config.h"
#include "dataset.h"
#include "gpio.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"

uint8_t ap_scan(AccessPoint aps[])
{
    uint16_t ap_count = APS_SIZE;
    wifi_ap_record_t ap_info[APS_SIZE];
    memset(ap_info, 0, sizeof(ap_info));

    static const wifi_scan_config_t scan_config = {
        .ssid = (unsigned char *)SSID,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active = {
            .min = 20,
            .max = 100,
        },
    };

    esp_wifi_scan_start(&scan_config, true);

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));

    for (int i = 0; i < ap_count; i++) {
        memcpy(&aps[i].mac, &ap_info[i].bssid, sizeof(aps[i].mac));
        aps[i].rssi = ap_info[i].rssi;
        print_ap(&aps[i]);
    }

    esp_wifi_scan_stop();

    return ap_count;
}
