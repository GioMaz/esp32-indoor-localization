#include "scan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"

StaticTask_t ap_scan_tcb;
StackType_t ap_scan_stack[AP_SCAN_STACK_SIZE];

void ap_scan_code(void *params)
{
    // TODO: read (x, y) from queue then scan
}

static void print_ap(AccessPoint *ap)
{
    printf("SSID: %s, RSSI: %d, MAC: %x:%x:%x:%x:%x:%x\n", ap->ssid, ap->rssi, ap->mac[0], ap->mac[1],
           ap->mac[2], ap->mac[3], ap->mac[4], ap->mac[5]);
}

uint16_t ap_scan(AccessPoint aps[])
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
    printf("Total APs scanned = %u, actual AP number ap_info holds = %u\n",
           ap_count, number);

    for (int i = 0; i < number; i++) {
        memcpy(&aps[i].ssid, &ap_info[i].ssid, sizeof(aps[i].ssid));
        memcpy(&aps[i].mac, &ap_info[i].bssid, sizeof(aps[i].mac));
        aps[i].rssi = ap_info[i].rssi;

        print_ap(&aps[i]);
        // printf("SSID \t\t%s\n", ap_info[i].ssid);
        // printf("RSSI \t\t%d\n", ap_info[i].rssi);
        // printf("Channel \t\t%d\n", ap_info[i].primary);
    }

    esp_wifi_scan_stop();

    return ap_count;
}

TaskHandle_t ap_scan_create()
{
    TaskHandle_t handle =
        xTaskCreateStatic(ap_scan_code, "ap_scan", AP_SCAN_STACK_SIZE, NULL,
                          tskIDLE_PRIORITY, ap_scan_stack, &ap_scan_tcb);
    return handle;
}
