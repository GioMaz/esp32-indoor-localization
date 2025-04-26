#include "network.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_wifi.h"

static const char *SSID = "Giovanni’s iPhone";

static void print_ap(AccessPoint *ap)
{
    printf("RSSI: %d, %x:%x:%x:%x:%x:%x\n", ap->rssi, ap->mac[0], ap->mac[1], ap->mac[2],
           ap->mac[3], ap->mac[4], ap->mac[5]);
}

void setup_wifi(void)
{
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_scan(AccessPoint aps[], uint32_t *ap_count)
{
    // Initialize variables
    wifi_ap_record_t ap_info[MAX_AP_LIST_SIZE];
    uint16_t number = MAX_AP_LIST_SIZE;
    memset(ap_info, 0, sizeof(ap_info));

    // Scan for acces points
    esp_err_t err = esp_wifi_scan_start(NULL, true);
    printf("%s\n", esp_err_to_name(err));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

    *ap_count = 0;
    for (int32_t i = 0; i < number; i++)
    {
        if (strcmp((char *)ap_info[i].ssid, SSID) == 0)
        {
            aps[*ap_count].rssi = ap_info[i].rssi,
            memcpy(&aps[*ap_count].mac, ap_info[i].bssid, sizeof(ap_info[i].bssid));
            print_ap(&aps[*ap_count]);
            (*ap_count)++;
        }
    }

    esp_wifi_scan_stop();

    /**/
    /*printf("GOT: %ld %s\n", *ap_count, SSID);*/
    /**/
    /*Features features_set[MAX_AP_LIST_SIZE];*/
    /*PreprocData preproc_data;*/
    /*aps_to_features_set(aps, features_set, (uint64_t) *ap_count,
     * &preproc_data);*/
}
