idf_component_register(
    SRCS "http_server.c" "routes.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES utils
    REQUIRES esp_http_server storage dataset
)
