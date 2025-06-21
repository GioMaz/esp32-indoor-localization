#include "config.h"
#include "esp_task_wdt.h"
#include "gpio.h"
#include "wifi.h"
#include "nvs.h"
#include "storage.h"

void setup(void)
{
    // Stop WatchDog Timer for the current task
    esp_task_wdt_deinit();

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
