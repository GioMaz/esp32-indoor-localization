idf_component_register(
    SRCS "state_inference.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES ap_scan utils inference
)
