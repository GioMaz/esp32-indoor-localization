idf_component_register(
    SRCS "console.c" "network.c" "nvs.c" "setup.c" "storage.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES littlefs common esp_wifi nvs_flash esp_driver_uart console config
)
