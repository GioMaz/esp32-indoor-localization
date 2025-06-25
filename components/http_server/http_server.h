#pragma once

#include "utils.h"
#include "dataset.h"
#include "esp_http_server.h"
#include "freertos/idf_additions.h"

/**
 * @brief Server context structure
 *
 * Contains information about the server's context, such as the current position
 * (x, y) that is periodically updated.
 */
typedef struct {
    Pos position; /**< Current position of the user */
    Dataset *dataset;
    QueueHandle_t state_queue;
    State current_state;
} server_context_t;

/**
 * @brief Update task arguments structure
 *
 * Contains the server context and the queue used for communication between
 * tasks, specifically for updating the server's position.
 */
typedef struct {
    server_context_t *ctx;        /**< Pointer to the server context */
    QueueHandle_t position_queue; /**< Queue to receive position updates */
} update_task_args_t;

/**
 * @brief Structure that encapsulates the HTTP server, context, and tasks
 *
 * Manages resources related to the HTTP server, the server context, and the
 * position update task.
 */
typedef struct {
    httpd_handle_t server;         /**< HTTP server handle */
    server_context_t *ctx;         /**< Server context */
    update_task_args_t *task_args; /**< Arguments for the update task */
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
ServerWrapper *http_server_start(QueueHandle_t position_queue,
                                 QueueHandle_t state_queue,
                                 const Dataset *dataset);

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
