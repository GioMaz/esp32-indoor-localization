#include "http_server.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"

static const char *TAG = "http_server";

void server_update_task(void *param)
{
    update_task_args_t *args = (update_task_args_t *)param;
    server_context_t *ctx = args->ctx;
    QueueHandle_t queue = args->queue;

    Pos new_position;

    while (1) {
        if (xQueueReceive(queue, &new_position, portMAX_DELAY)) {
            ctx->position = new_position;
        }
    }
}

esp_err_t get_position_handler(httpd_req_t *req)
{
    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

    float x = ctx->position.x;
    float y = ctx->position.y;

    char json_response[64];
    snprintf(json_response, sizeof(json_response), "{\"x\": %.2f, \"y\": %.2f}", x, y);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

ServerWrapper *http_server_start(QueueHandle_t queue)
{
    ServerWrapper *server_wrapper = calloc(1, sizeof(ServerWrapper));
    if (!server_wrapper) {
        ESP_LOGE(TAG, "Failed to allocate HttpServer struct");
        return NULL;
    }

    server_wrapper->ctx = calloc(1, sizeof(server_context_t));
    if (!server_wrapper->ctx) {
        ESP_LOGE(TAG, "Failed to allocate context");
        free(server_wrapper);
        return NULL;
    }
    server_wrapper->ctx->position = (Pos){.x = 0, .y = 0};

    server_wrapper->task_args = calloc(1, sizeof(update_task_args_t));
    if (!server_wrapper->task_args) {
        ESP_LOGE(TAG, "Failed to allocate task args");
        free(server_wrapper->ctx);
        free(server_wrapper);
        return NULL;
    }

    server_wrapper->task_args->ctx = server_wrapper->ctx;
    server_wrapper->task_args->queue = queue;

    if (xTaskCreate(server_update_task, "server_update", 2048, server_wrapper->task_args, 5,
                    &server_wrapper->task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task");
        free(server_wrapper->task_args);
        free(server_wrapper->ctx);
        free(server_wrapper);
        return NULL;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server_wrapper->server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        vTaskDelete(server_wrapper->task_handle);
        free(server_wrapper->task_args);
        free(server_wrapper->ctx);
        free(server_wrapper);
        return NULL;
    }

    ESP_LOGI(TAG, "Http Server started on port: [%d]", config.server_port);

    httpd_uri_t get_position = {.uri = "/api/position",
                                .method = HTTP_GET,
                                .handler = get_position_handler,
                                .user_ctx = server_wrapper->ctx};

    httpd_register_uri_handler(server_wrapper->server, &get_position);

    return server_wrapper;
}

void http_server_stop(ServerWrapper *server_wrapper)
{
    if (!server_wrapper)
        return;

    if (server_wrapper->server) {
        httpd_stop(server_wrapper->server);
    }

    if (server_wrapper->task_handle) {
        vTaskDelete(server_wrapper->task_handle);
    }

    free(server_wrapper->task_args);
    free(server_wrapper->ctx);
    free(server_wrapper);

    ESP_LOGI(TAG, "Http Server stopped");
}
