#pragma once

#include "dataset.h"
#include "esp_http_server.h"
#include "freertos/idf_additions.h"
#include "utils.h"
#include <stdbool.h>

/**
 * @brief Server context structure
 *
 * Contains information about the server's context, such as the current position
 * (x, y) that is periodically updated.
 */
typedef struct {
    Dataset *dataset;
    State *state;
    Pos *pos; /**< Current position of the user */
} server_context_t;

/**
 * @brief Structure that encapsulates the HTTP server, context, and tasks
 *
 * Manages resources related to the HTTP server, the server context, and the
 * position update task.
 */
typedef struct {
    httpd_handle_t server;         /**< HTTP server handle */
    server_context_t *ctx;         /**< Server context */
    TaskHandle_t task_handle;      /**< Handle for the update task */
} ServerWrapper;

/**
 * @brief Starts the HTTP server
 *
 * Initializes and starts the HTTP server on the default port. It also registers
 * the handler for the GET request that provides the server's position.
 * Additionally, it creates a task that periodically updates the server's
 * position via the queue.
 *
 * @param queue The queue used to receive position updates
 *
 * @return A pointer to a `ServerWrapper` object if the start is successful,
 *         otherwise `NULL`.
 */
ServerWrapper *http_server_start(Dataset *dataset, State *state, Pos *pos);

/**
 * @brief Stops the HTTP server
 *
 * Stops the running HTTP server and frees all associated resources, including
 * the HTTP server, the update task, and the memory allocated for the context
 * and task arguments.
 *
 * @param server The pointer to the `ServerWrapper` structure containing the
 *               server and associated resources to stop and free.
 */
void http_server_stop(ServerWrapper *server);
