#include "http_server.h"
#include "common.h"

/* Handler function to be called during GET /api/position request */
esp_err_t get_position_handler(httpd_req_t *req)
{
    //--- get data from ipc
    float x = 12.34;
    float y = 56.78;
    //---

    char json_response[64];
    snprintf(json_response, sizeof(json_response), "{\"x\": %.2f, \"y\": %.2f}", x, y);

    httpd_resp_set_type(req, "application/json");

    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for GET /api/position */
httpd_uri_t uri_get = {
    .uri = "/api/position", .method = HTTP_GET, .handler = get_position_handler, .user_ctx = NULL};

httpd_handle_t http_server_start(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

void http_server_stop(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}
