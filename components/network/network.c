#include "network.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"

/*static void print_ap(AccessPoint *ap)*/
/*{*/
/*    printf("RSSI: %d, %x:%x:%x:%x:%x:%x\n", ap->rssi, ap->mac[0], ap->mac[1], ap->mac[2],*/
/*           ap->mac[3], ap->mac[4], ap->mac[5]);*/
/*}*/

static const char *TAG = "network";

#define AP_WIFI_SSID "martin_router_king"
#define AP_WIFI_PASS "ciao123456"
#define AP_WIFI_MAX_STA_CONN 3

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                               void *event_data)
{
    if (event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "Access Point started");
    } else if (event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "Access Point stopped");
    }
}

void setup_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));
}

void ap_start() {
    ESP_LOGI(TAG, "Starting Access Point...");

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_WIFI_SSID,
            .password = AP_WIFI_PASS,
            .max_connection = AP_WIFI_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void ap_stop() {
    ESP_LOGI(TAG, "Stopping Access Point...");

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
}
