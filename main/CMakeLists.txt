idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES core common network esp_wifi nvs_flash console esp_driver_uart http_server storage
)
