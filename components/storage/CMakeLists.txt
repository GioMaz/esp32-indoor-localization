idf_component_register(
    SRCS "storage.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES littlefs
    REQUIRES dataset
)
