#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "nvs_flash.h"

static const uint32_t MAX_AP_LIST_SIZE = 20;
static const uint64_t TIMEOUT = 1e6;
static const char *SSID = "unitn-x";

static bool should_scan = true;

typedef struct {
    /*int8_t channel;*/
    char mac_addr[6];
    int8_t rssi;
} AccesPoint;

typedef double Features[2];

// Represents an offset from the initial position (0, 0) (in meters)
typedef struct {
    int16_t x, y;
} Vec2;

static void aps_to_features_vec(AccesPoint ap[], Features features, size_t count)
{
}

static void print_ap(AccesPoint *ap)
{
    printf("RSSI: %d, %x:%x:%x:%x:%x:%x\n",
           ap->rssi,
           ap->mac_addr[0],
           ap->mac_addr[1],
           ap->mac_addr[2],
           ap->mac_addr[3],
           ap->mac_addr[4],
           ap->mac_addr[5]
    );
}

static void callback(void *arg)
{
    should_scan = true;
}

static void setup_timer(void)
{
    const esp_timer_create_args_t args = {
        .callback = callback,
        .arg = NULL,
        .name = "Scan timer",
    };

    esp_timer_handle_t timer;

    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));

    esp_timer_start_periodic(timer, TIMEOUT);
}

static void setup_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static void setup(void)
{
    // Stop WatchDog Timer for the current task
    esp_task_wdt_deinit();

    // Setup timer
    setup_timer();

    // Initialize Non-Volatile Storage
    setup_nvs();

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Initialize event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize wifi station
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

}

static void wifi_scan(AccesPoint aps[], uint16_t *ap_count)
{
    // Initialize variables
    wifi_ap_record_t ap_info[MAX_AP_LIST_SIZE];
    uint16_t number = MAX_AP_LIST_SIZE;
    memset(ap_info, 0, sizeof(ap_info));

    // Scan for acces points
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

    *ap_count = 0;
    for (int i = 0; i < number; i++) {
        if (strcmp((char *)ap_info[i].ssid, SSID) == 0) {
            aps[*ap_count].rssi = ap_info[i].rssi,
            memcpy(&aps[*ap_count], ap_info[i].bssid, sizeof(ap_info[i].bssid));
            print_ap(&aps[*ap_count]);
            (*ap_count)++;
        }
    }
    printf("GOT: %d %s\n", *ap_count, SSID);
}

void app_main(void)
{
    setup();

    // Loop
    while (1) {
        if (should_scan) {
            AccesPoint aps[MAX_AP_LIST_SIZE];
            uint16_t ap_count;
            wifi_scan(aps, &ap_count);
            should_scan = false;
        }
    }
}
