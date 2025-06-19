idf_component_register(
    SRCS "setup.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES repl nvs wifi storage gpio config
)
