#pragma once
#include "esp_http_server.h"

/**
 * @brief Starts the HTTP server
 *
 * Initializes and starts the HTTP server on the default port.
 * The server is configured with necessary handlers to handle incoming requests.
 *
 * @return HTTP server handle if the start is successful, otherwise NULL.
 */
httpd_handle_t http_server_start(void);

/**
 * @brief Stops the HTTP server
 *
 * Stops the running HTTP server and frees any associated resources.
 *
 * @param server The handle of the server to stop.
 */
void http_server_stop(httpd_handle_t server);
