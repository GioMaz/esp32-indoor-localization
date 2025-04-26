#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "network.h"

#define CONSOLE 1

#ifdef CONSOLE
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#endif

/*static const uint32_t TIMEOUT = 1e6;*/
static const uint32_t MAX_DATAPOINTS = 1024;
static const uint32_t CMD_SIZE = 16;
/*static const char *SSID = "unitn-x";*/

static bool should_scan = true;

static void setup_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static void setup_console(void)
{
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    /*esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
     * ESP_LINE_ENDINGS_CR);*/
    /* Move the caret to the beginning of the next line on '\n' */
    /*esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
     * ESP_LINE_ENDINGS_CRLF);*/

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
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {.max_cmdline_args = 8,
                                           .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
                                           .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback *)&esp_console_get_hint);

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

void app_main(void)
{
    setup();

    AccessPoint total_aps[MAX_DATAPOINTS];
    Label total_labels[MAX_DATAPOINTS];

    FeaturesLabel fls[MAX_DATAPOINTS];

    uint32_t count = 0;

#ifdef CONSOLE

    // Loop
    while (1)
    {
        char *line = linenoise("> ");

        char cmd[CMD_SIZE];
        sscanf(line, "%s ", cmd);

        if (strcmp(cmd, "reg") == 0)
        {
            int32_t x, y;
            sscanf(line, "%*s %ld %ld\n", &x, &y);
            linenoiseFree(line);

            printf("TRYING %ld %ld...\n", x, y);

            AccessPoint aps[MAX_AP_LIST_SIZE];
            uint32_t ap_count;
            wifi_scan(aps, &ap_count);

            for (uint32_t i = 0; i < ap_count; i++)
            {
                if (count == MAX_DATAPOINTS)
                {
                    exit(1);
                }
                total_aps[count] = aps[i];
                total_labels[count] = (Label){.x = x, .y = y};
                count++;
            }
        }
        else if (strcmp(cmd, "listen") == 0)
        {
            printf("LISTENING...\n");
            while (1)
                ;
        }
        else if (strcmp(cmd, "quit") == 0)
        {
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
