#include "scan.h"
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

    esp_wifi_scan_start(NULL, true);

    // ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    printf("Max scanned APs = %u\n", ap_count);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));
    printf("Scanned APs = %u\n", ap_count);

    uint8_t j = 0;
    for (int i = 0; i < ap_count; i++) {
        if (strcmp((const char *)ap_info[i].ssid, SSID) == 0) {
            memcpy(&aps[j].mac, &ap_info[i].bssid, sizeof(aps[j].mac));
            aps[j].rssi = ap_info[i].rssi;
            print_ap(&aps[j]);
            j++;
        }
    }

    esp_wifi_scan_stop();

    return j;
}
