idf_component_register(
    SRCS "http_server.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES common
    REQUIRES esp_http_server
)
