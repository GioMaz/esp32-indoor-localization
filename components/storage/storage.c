#include "storage.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "storage";

static const esp_vfs_littlefs_conf_t conf = {
    .base_path = "/littlefs",
    .partition_label = "storage",
    .format_if_mount_failed = true,
    .dont_mount = false,
};

void mount_storage()
{
    ESP_LOGI(TAG, "Mounting storage (LittleFS)...");

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)",
                     esp_err_to_name(ret));
        }
        return;
    }

    ESP_LOGI(TAG, "Storage (LittleFS) mounted at %s", conf.base_path);
}

void unmount_storage()
{
    ESP_LOGI(TAG, "Unmounting storage (LittleFS)...");
    esp_vfs_littlefs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "Storage (LittleFS) unmounted");
}

int read_dataset_from_storage(Dataset *dataset)
{

    char filepath[PATH_LEN] = {0};
    snprintf(filepath, sizeof(filepath), "%s/dataset.bin", BASE_PATH);

    FILE *file = fopen(filepath, "r");
    if (!file) {
        return 1;
    }

    return fread((char *)dataset, 1, sizeof(Dataset), file) == 0;
}
