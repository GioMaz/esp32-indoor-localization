#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t get_state_handler(httpd_req_t *req);
esp_err_t get_dataset_handler(httpd_req_t *req);
esp_err_t static_file_handler(httpd_req_t *req);
esp_err_t post_switch_state_handler(httpd_req_t *req);
esp_err_t get_map_handler(httpd_req_t *req);
esp_err_t post_reset_dataset_handler(httpd_req_t *req);
