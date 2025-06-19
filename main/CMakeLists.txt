idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES setup inference scan gpio utils esp_wifi http_server console config driver
)
