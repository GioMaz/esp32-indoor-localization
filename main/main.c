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
/*static const char *SSID = "unitn-x";*/
static const char *SSID = "Giovanni’s iPhone";

static bool should_scan = true;

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

typedef struct {
    /*int8_t channel;*/
    char mac_addr[6];
    int8_t rssi;
} AccesPoint;

typedef double Features[2];

typedef struct {
    uint64_t min_mac, max_mac;
    int8_t min_rssi, max_rssi;
} PreprocData;

typedef struct {
    int16_t x, y;
} Vec2;

static void aps_to_features_vec(AccesPoint aps[], Features features_vec[], uint64_t count, PreprocData *preproc_data)
{
    uint64_t min_mac = UINT64_MAX;
    uint64_t max_mac = 0;
    int8_t min_rssi = INT8_MAX;
    int8_t max_rssi = INT8_MIN;

    for (uint32_t i = 0; i < count; i++) {
        uint64_t mac = *((uint64_t *)aps[i].mac_addr);
        min_mac = MIN(min_mac, mac);
        max_mac = MAX(max_mac, mac);

        int8_t rssi = aps[i].rssi;
        min_rssi = MIN(min_rssi, rssi);
        max_rssi = MAX(max_rssi, rssi);
    }

    double diff_mac = (double) (max_mac - min_mac);
    double diff_rssi = (double) (max_rssi - min_rssi);

    if (diff_mac == 0) diff_mac = 1;
    if (diff_rssi == 0) diff_rssi = 1;

    for (uint32_t i = 0; i < count; i++) {
        uint64_t mac = *((uint64_t *)aps[i].mac_addr);
        int8_t rssi = aps[i].rssi;
        features_vec[i][0] = ((double) (mac - min_mac)) / diff_mac;
        features_vec[i][1] = ((double) (rssi - min_rssi)) / diff_rssi;
        printf("FEATURE %lu: %lf %lf\n", i, features_vec[i][0], features_vec[i][1]);
    }

    printf("min_mac: %llu\n", min_mac);
    printf("max_mac: %llu\n", max_mac);
    printf("min_rssi: %d\n", min_rssi);
    printf("max_rssi: %d\n", max_rssi);

    preproc_data->min_mac = min_mac;
    preproc_data->max_mac = max_mac;
    preproc_data->min_rssi = min_rssi;
    preproc_data->max_rssi = max_rssi;
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

// TODO: make this an interrupt handler for a button press
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

static void setup_wifi(void)
{
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
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
    setup_wifi();
}

static void wifi_scan(AccesPoint aps[], uint16_t *ap_count)
{
    // Initialize variables
    wifi_ap_record_t ap_info[MAX_AP_LIST_SIZE];
    uint16_t number = MAX_AP_LIST_SIZE;
    memset(ap_info, 0, sizeof(ap_info));

    // Scan for acces points
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

    *ap_count = 0;
    for (int32_t i = 0; i < number; i++) {
        if (strcmp((char *)ap_info[i].ssid, SSID) == 0) {
            aps[*ap_count].rssi = ap_info[i].rssi,
            memcpy(&aps[*ap_count], ap_info[i].bssid, sizeof(ap_info[i].bssid));
            print_ap(&aps[*ap_count]);
            (*ap_count)++;
        }
    }

    printf("GOT: %d %s\n", *ap_count, SSID);

    Features features_vec[MAX_AP_LIST_SIZE];
    PreprocData preproc_data;
    aps_to_features_vec(aps, features_vec, (uint64_t) *ap_count, &preproc_data);
}

void app_main(void)
{
    setup();

    // Set initial position
    Vec2 pos = { 0, 0 };

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
