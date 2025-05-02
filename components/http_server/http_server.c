#include "http_server.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

httpd_handle_t http_server_start(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        // ...
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
