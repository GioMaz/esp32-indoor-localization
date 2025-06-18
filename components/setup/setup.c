#include "config.h"
#include "esp_task_wdt.h"
#include "gpio.h"
#include "network.h"
#include "nvs.h"
#include "storage.h"

#ifdef CONSOLE
#include "console.h"
#endif

void setup(void)
{
    // Stop WatchDog Timer for the current task
    esp_task_wdt_deinit();

    // Setup console
#ifdef CONSOLE
    setup_console();
#endif

    // Initialize Non-Volatile Storage
    setup_nvs();

    // Initialize wifi station
    setup_wifi();

    // Initialize little fs
    mount_storage();

    // Start access point
    ap_start();

    // Setup gpio
    setup_gpio();
}
