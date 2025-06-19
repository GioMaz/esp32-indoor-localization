idf_component_register(
    SRCS "scan.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES common gpio config esp_wifi setup
    REQUIRES dataset
)
