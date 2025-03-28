#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"

#define CONSOLE 1

#ifdef CONSOLE
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#endif

static const uint32_t MAX_AP_LIST_SIZE = 20;
/*static const uint32_t TIMEOUT = 1e6;*/
static const uint32_t MAX_DATAPOINTS = 1024;
static const uint32_t CMD_SIZE = 16;
/*static const char *SSID = "unitn-x";*/
static const char *SSID = "Giovanni’s iPhone";

static bool should_scan = true;

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

typedef struct {
    /*int8_t channel;*/
    uint8_t mac[6];
    int8_t rssi;
} AccessPoint;

/*typedef double Features[2];*/
typedef struct {
    double x, y;
} Features;

typedef struct {
    int16_t x, y;
} Label;

typedef struct {
    Features features;
    Label label;
} FeaturesLabel;

typedef struct {
    uint64_t min_mac, max_mac;
    int8_t min_rssi, max_rssi;
} PreprocData;

static uint64_t mac_to_feature(const uint8_t mac[6])
{
    uint8_t feature[8] = {0};

    // ESP32 uses little endian
    feature[7] = mac[0];
    feature[6] = mac[1];
    feature[5] = mac[2];
    feature[4] = mac[3];
    feature[3] = mac[4];
    feature[2] = mac[5];

    return (uint64_t)feature;
}

static double euclidean_dist(const Features *f_1, const Features *f_2)
{
    double x = f_2->x - f_1->x;
    double y = f_2->y - f_1->y;
    return sqrt(x * x + y * y);
}

static int cmp(const void *fl_1, const void *fl_2, void *query)
{
    Features *f_1 = &((FeaturesLabel *)fl_1)->features;
    Features *f_2 = &((FeaturesLabel *)fl_2)->features;
    double dist_1 = euclidean_dist((Features *)query, f_1);
    double dist_2 = euclidean_dist((Features *)query, f_2);
    return (int) (dist_1 - dist_2);
}

static void ap_to_features(const PreprocData *pd, const AccessPoint *ap, Features *features)
{
    uint64_t mac = mac_to_feature(ap->mac);
    int8_t rssi = ap->rssi;

    double diff_mac = (double)(pd->max_mac - pd->min_mac);
    double diff_rssi = (double)(pd->max_rssi - pd->min_rssi);

    if (diff_mac == 0) diff_mac = 1;
    if (diff_rssi == 0) diff_rssi = 1;

    features->x = ((double)(mac - pd->min_mac)) / diff_mac;
    features->y =((double)(rssi - pd->min_rssi)) / diff_rssi;
}

static Label knn(FeaturesLabel fl_set[], uint32_t count, uint32_t k, Features *query)
{
    qsort_r(fl_set, count, sizeof(FeaturesLabel), cmp, query);
    uint64_t n = MIN(k, count);
    double x_mean = 0;
    double y_mean = 0;
    for (uint32_t i = 0; i < n; i++) {
        x_mean += (double)fl_set[i].label.x;
        y_mean += (double)fl_set[i].label.y;
    }
    x_mean /= (double)n;
    y_mean /= (double)n;
    return (Label) { .x = x_mean, .y = y_mean };
}

static void aps_to_features_set(const AccessPoint aps[], Features features_set[], uint64_t count, PreprocData *preproc_data)
{
    uint64_t min_mac = UINT64_MAX;
    uint64_t max_mac = 0;
    int8_t min_rssi = INT8_MAX;
    int8_t max_rssi = INT8_MIN;

    for (uint32_t i = 0; i < count; i++) {
        uint64_t mac = mac_to_feature(aps[i].mac);
        min_mac = MIN(min_mac, mac);
        max_mac = MAX(max_mac, mac);

        int8_t rssi = aps[i].rssi;
        min_rssi = MIN(min_rssi, rssi);
        max_rssi = MAX(max_rssi, rssi);
    }

    preproc_data->min_mac = min_mac;
    preproc_data->max_mac = max_mac;
    preproc_data->min_rssi = min_rssi;
    preproc_data->max_rssi = max_rssi;

    printf("min_mac: %llu\n", min_mac);
    printf("max_mac: %llu\n", max_mac);
    printf("min_rssi: %d\n", min_rssi);
    printf("max_rssi: %d\n", max_rssi);

    for (uint32_t i = 0; i < count; i++) {
        ap_to_features(preproc_data, &aps[i], &features_set[i]);
        printf("FEATURES %lu: %lf %lf\n", i, features_set[i].x, features_set[i].y);
    }
}

static void print_ap(AccessPoint *ap)
{
    printf("RSSI: %d, %x:%x:%x:%x:%x:%x\n",
           ap->rssi,
           ap->mac[0],
           ap->mac[1],
           ap->mac[2],
           ap->mac[3],
           ap->mac[4],
           ap->mac[5]
    );
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

static void setup_console(void)
{
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    /*esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);*/
    /* Move the caret to the beginning of the next line on '\n' */
    /*esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);*/

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
#if SOC_UART_SUPPORT_REF_TICK
        .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
        .source_clk = UART_SCLK_XTAL,
#endif
    };

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);
}

static void setup(void)
{
    // Stop WatchDog Timer for the current task
    esp_task_wdt_deinit();

    // Setup console
#ifdef CONSOLE
    setup_console();
#endif

    // Initialize Non-Volatile Storage
    setup_nvs();

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Initialize event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize wifi station
    setup_wifi();
}

static void wifi_scan(AccessPoint aps[], uint32_t *ap_count)
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
    for (int32_t i = 0; i < number; i++) {
        if (strcmp((char *)ap_info[i].ssid, SSID) == 0) {
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
    /*aps_to_features_set(aps, features_set, (uint64_t) *ap_count, &preproc_data);*/
}

void app_main(void)
{
    setup();

    AccessPoint total_aps[MAX_DATAPOINTS];
    Label total_labels[MAX_DATAPOINTS];

    FeaturesLabel fls[MAX_DATAPOINTS];

    uint32_t count = 0;

#ifdef CONSOLE

    // Loop
    while (1) {
        char* line = linenoise("> ");

        char cmd[CMD_SIZE];
        sscanf(line, "%s ", cmd);

        if (strcmp(cmd, "reg") == 0) {
            int32_t x, y;
            sscanf(line, "%*s %ld %ld\n", &x, &y);
            linenoiseFree(line);

            printf("TRYING %ld %ld...\n", x, y);

            AccessPoint aps[MAX_AP_LIST_SIZE];
            uint32_t ap_count;
            wifi_scan(aps, &ap_count);

            for (uint32_t i = 0; i < ap_count; i++) {
                if (count == MAX_DATAPOINTS) {
                    exit(1);
                }
                total_aps[count] = aps[i];
                total_labels[count] = (Label) { .x = x, .y = y };
                count++;
            }
        } else if (strcmp(cmd, "listen") == 0) {
            printf("LISTENING...\n");
            while (1);
        } else if (strcmp(cmd, "quit") == 0) {
            exit(0);
        }

        /*if (should_scan) {*/
        /*    AccessPoint aps[MAX_AP_LIST_SIZE];*/
        /*    uint16_t ap_count;*/
        /*    wifi_scan(aps, &ap_count);*/
        /*    should_scan = false;*/
        /*}*/
    }

#endif
}
