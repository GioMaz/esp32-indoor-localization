idf_component_register(
    SRCS "scan.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES common esp_wifi setup
)
