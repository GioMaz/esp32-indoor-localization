idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES setup state_training state_inference scan gpio utils esp_wifi http_server console config driver
)
