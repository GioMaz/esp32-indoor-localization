#include "scan.h"
#include "config.h"
#include "gpio.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"

StaticTask_t ap_scan_tcb;
StackType_t ap_scan_stack[AP_SCAN_STACK_SIZE];

Pos dir_to_offset[] = {
    [LEFT] = {-1, 0},
    [RIGHT] = {1, 0},
    [UP] = {0, 1},
    [DOWN] = {0, -1},
};

void ap_scan_code(void *params)
{
    // Destruct params
    ScanParams *scan_params = (ScanParams *)params;
    QueueHandle_t scan_queue = scan_params->scan_queue;
    AccessPoint *total_aps = scan_params->total_aps;
    Pos *total_labels = scan_params->total_labels;

    // Setup variables
    Direction direction;
    Pos position = {0, 0};
    uint32_t count = 0;

    while (1) {
        if (xQueueReceive(scan_queue, &direction, portMAX_DELAY)) {
            // Block if max datapoints reached
            if (count < MAX_DATAPOINTS) {
                // Apply direction
                position.x += dir_to_offset[direction].x;
                position.y += dir_to_offset[direction].y;
                printf("Scanning position (%d, %d)\n", position.x, position.y);

                // Create temporary datapoints
                AccessPoint aps[SCAN_SIZE];

                // Scan datapoints
                uint16_t ap_count = ap_scan(aps);

                // Copy scanned datapoints to dataset
                int i = 0;
                while (i < ap_count && count < MAX_DATAPOINTS) {
                    memcpy(&total_aps[count], &aps[i], sizeof(total_aps[count]));
                    total_labels[count] = position;
                    i++;
                    count++;
                }
            }

            if (count == MAX_DATAPOINTS) {
                printf("ERROR: Max number of datapoints reached\n");
            }
        }
    }
}

static void print_ap(AccessPoint *ap)
{
    printf("RSSI: %d, MAC: %x:%x:%x:%x:%x:%x\n", ap->rssi, ap->mac[0],
           ap->mac[1], ap->mac[2], ap->mac[3], ap->mac[4], ap->mac[5]);
}

uint16_t ap_scan(AccessPoint aps[])
{
    uint16_t number = SCAN_SIZE;
    wifi_ap_record_t ap_info[SCAN_SIZE];
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
        if (strcmp((const char *)ap_info[i].ssid, SSID) == 0) {
            // memcpy(&aps[i].ssid, &ap_info[i].ssid, sizeof(aps[i].ssid));
            memcpy(&aps[i].mac, &ap_info[i].bssid, sizeof(aps[i].mac));
            aps[i].rssi = ap_info[i].rssi;

            print_ap(&aps[i]);
        }
        // printf("SSID \t\t%s\n", ap_info[i].ssid);
        // printf("RSSI \t\t%d\n", ap_info[i].rssi);
        // printf("Channel \t\t%d\n", ap_info[i].primary);
    }

    esp_wifi_scan_stop();

    return ap_count;
}

TaskHandle_t ap_scan_create(ScanParams *params)
{
    TaskHandle_t handle =
        xTaskCreateStatic(ap_scan_code, "ap_scan", AP_SCAN_STACK_SIZE, params,
                          tskIDLE_PRIORITY, ap_scan_stack, &ap_scan_tcb);
    return handle;
}
