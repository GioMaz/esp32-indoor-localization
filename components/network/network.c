#include "network.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_wifi.h"

/*static void print_ap(AccessPoint *ap)*/
/*{*/
/*    printf("RSSI: %d, %x:%x:%x:%x:%x:%x\n", ap->rssi, ap->mac[0], ap->mac[1], ap->mac[2],*/
/*           ap->mac[3], ap->mac[4], ap->mac[5]);*/
/*}*/

void setup_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

uint16_t wifi_scan(AccessPoint aps[])
{
    uint16_t number = MAX_APS;
    wifi_ap_record_t ap_info[MAX_APS];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_scan_start(NULL, true);

    printf("Max AP number ap_info can hold = %u\n", number);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    printf("Total APs scanned = %u, actual AP number ap_info holds = %u\n", ap_count, number);

    for (int i = 0; i < number; i++) {
        memcpy(&aps[i].ssid, &ap_info[i].ssid, sizeof(aps[i].ssid));
        memcpy(&aps[i].mac, &ap_info[i].bssid, sizeof(aps[i].mac));
        aps[i].rssi = ap_info[i].rssi;

        printf("SSID \t\t%s\n", ap_info[i].ssid);
        printf("RSSI \t\t%d\n", ap_info[i].rssi);
        printf("Channel \t\t%d\n", ap_info[i].primary);
    }

    esp_wifi_scan_stop();

    return ap_count;
}
