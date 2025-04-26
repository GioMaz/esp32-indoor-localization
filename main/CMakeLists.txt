idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES core network esp_wifi nvs_flash console esp_driver_uart
)
