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
    QueueHandle_t direction_queue = scan_params->direction_queue;
    Dataset *dataset = scan_params->dataset;

    // Setup variables
    Direction direction;
    Pos position = {0, 0};

    while (1) {
        if (xQueueReceive(direction_queue, &direction, portMAX_DELAY)) {
            // Block if max datapoints reached
            if (dataset->data_count < DATASET_SIZE) {
                // Apply direction
                position.x += dir_to_offset[direction].x;
                position.y += dir_to_offset[direction].y;
                printf("Scanning position (%d, %d)\n", position.x, position.y);

                // Create temporary datapoints
                AccessPoint aps[APS_SIZE];

                // Scan datapoints
                uint16_t ap_count = ap_scan(aps);

                // Copy scanned datapoints to dataset
                for (int i = 0; i < ap_count; i++) {
                    dataset_insert_ap(dataset, &aps[i], position);
                }
            }

            if (dataset->data_count == DATASET_SIZE) {
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
    uint16_t ap_count = APS_SIZE;
    wifi_ap_record_t ap_info[APS_SIZE];
    memset(ap_info, 0, sizeof(ap_info));

    esp_wifi_scan_start(NULL, true);

    // ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    printf("Max scanned APs = %u\n", ap_count);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));
    printf("Scanned APs = %u\n", ap_count);

    for (int i = 0; i < ap_count; i++) {
        if (strcmp((const char *)ap_info[i].ssid, SSID) == 0) {
            memcpy(&aps[i].mac, &ap_info[i].bssid, sizeof(aps[i].mac));
            aps[i].rssi = ap_info[i].rssi;

            print_ap(&aps[i]);
        }
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
