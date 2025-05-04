#include "http_server.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"

static const char * TAG = "http_server";

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

HttpServer *http_server_start(QueueHandle_t queue)
{
    HttpServer *http = calloc(1, sizeof(HttpServer));
    if (!http) {
        ESP_LOGE(TAG, "Failed to allocate HttpServer struct");
        return NULL;
    }

    http->ctx = calloc(1, sizeof(server_context_t));
    if (!http->ctx) {
        ESP_LOGE(TAG, "Failed to allocate context");
        free(http);
        return NULL;
    }
    http->ctx->position = (Pos){.x = 0, .y = 0};

    http->task_args = calloc(1, sizeof(update_task_args_t));
    if (!http->task_args) {
        ESP_LOGE(TAG, "Failed to allocate task args");
        free(http->ctx);
        free(http);
        return NULL;
    }

    http->task_args->ctx = http->ctx;
    http->task_args->queue = queue;

    if (xTaskCreate(server_update_task, "server_update", 2048, http->task_args, 5, &http->task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task");
        free(http->task_args);
        free(http->ctx);
        free(http);
        return NULL;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&http->server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        vTaskDelete(http->task_handle);
        free(http->task_args);
        free(http->ctx);
        free(http);
        return NULL;
    }

    ESP_LOGI(TAG, "Http Server started on port: [%d]", config.server_port);

    httpd_uri_t get_position = {
        .uri = "/api/position",
        .method = HTTP_GET,
        .handler = get_position_handler,
        .user_ctx = http->ctx};

    httpd_register_uri_handler(http->server, &get_position);

    return http;
}

void http_server_stop(HttpServer *http)
{
    if (!http) return;

    if (http->server) {
        httpd_stop(http->server);
    }

    if (http->task_handle) {
        vTaskDelete(http->task_handle);
    }

    free(http->task_args);
    free(http->ctx);
    free(http);

    ESP_LOGI(TAG, "Http Server stopped");
}
