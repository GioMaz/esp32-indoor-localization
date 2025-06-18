#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"

#include "http_server.h"
#include "core.h"
#include "scan.h"
#include "setup.h"
#include "config.h"
#include "common.h"

#ifdef CONSOLE
#include "linenoise/linenoise.h"
#endif

#define CMD_SIZE        16
#define SSID            "unitn-x"

void app_main(void)
{
    setup();

    // Setup scan task
    AccessPoint total_aps[MAX_DATAPOINTS];
    Pos total_labels[MAX_DATAPOINTS];
    // FeaturesLabel fls[MAX_DATAPOINTS];
    QueueHandle_t scan_queue = xQueueCreate(10, sizeof(Pos));
    ScanParams scan_params = (ScanParams) {
        .queue = scan_queue,
        total_aps,
        total_labels,
    };
    TaskHandle_t scan = ap_scan_create(&scan_params);

    // Setup server task
    QueueHandle_t server_queue = xQueueCreate(10, sizeof(Pos));
    ServerWrapper *server = http_server_start(server_queue);

#ifdef CONSOLE

    // Loop
    while (1) {
        // char *line = linenoise("> ");
        //
        // char cmd[CMD_SIZE];
        // sscanf(line, "%s ", cmd);
        //
        // if (strcmp(cmd, "reg") == 0) {
        //     int32_t x, y;
        //     sscanf(line, "%*s %ld %ld\n", &x, &y);
        //
        //     printf("TRYING %ld %ld...\n", x, y);
        //
        //     AccessPoint aps[MAX_APS];
        //     uint16_t ap_count = ap_scan(aps);
        //     printf("DONE %d\n", ap_count);
        //
        //     int i = 0;
        //     while (i < ap_count && count < MAX_DATAPOINTS) {
        //         memcpy(&total_aps[count], &aps[i], sizeof(total_aps[count]));
        //         total_labels[count] = (Pos){x, y};
        //         i++; count++;
        //     }
        //
        //     if (count == MAX_DATAPOINTS) {
        //         printf("ERROR: Max number of datapoints reached\n");
        //         while (1)
        //             ;
        //     }
        // } else if (strcmp(cmd, "listen") == 0) {
        //     printf("Listening...\n");
        //     while (1)
        //         ;
        // } else if (strcmp(cmd, "quit") == 0) {
        //     printf("Completed...\n");
        //     while (1)
        //         ;
        // }
        //
        // linenoiseFree(line);
    }

#endif

    // http_server_stop(server);
    // ap_stop();
    // unmount_storage();
}
