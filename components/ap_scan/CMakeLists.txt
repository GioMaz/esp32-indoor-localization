idf_component_register(
    SRCS "ap_scan.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES utils gpio config esp_wifi setup driver
    REQUIRES dataset
)
