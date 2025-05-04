#pragma once

#include "common.h"
#include "esp_http_server.h"
#include "freertos/idf_additions.h"

typedef struct {
    Label position;
} server_context_t;

typedef struct {
    server_context_t *ctx;
    QueueHandle_t queue;
} update_task_args_t;

typedef struct {
    httpd_handle_t server;
    server_context_t *ctx;
    update_task_args_t *task_args;
    TaskHandle_t task_handle;
} HttpServer;

/**
 * @brief Starts the HTTP server
 *
 * Initializes and starts the HTTP server on the default port.
 * The server is configured with necessary handlers to handle incoming requests.
 *
 * @param queue
 *
 * @return HTTP server handle if the start is successful, otherwise NULL.
 */
HttpServer * http_server_start(QueueHandle_t queue);

/**
 * @brief Stops the HTTP server
 *
 * Stops the running HTTP server and frees any associated resources.
 *
 * @param server The handle of the server to stop.
 */
void http_server_stop(HttpServer * server);
