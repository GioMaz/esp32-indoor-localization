#include "http_server.h"
#include "dataset.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "routes.h"

static const char *TAG = "http_server";

ServerWrapper *http_server_start(Dataset *dataset, State *state, Pos *pos)
{
    ESP_LOGI(TAG, "Starting Http Server...");

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
    server_wrapper->ctx->dataset = dataset;
    server_wrapper->ctx->state = state;
    server_wrapper->ctx->pos = pos;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server_wrapper->server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        free(server_wrapper->ctx);
        free(server_wrapper);
        return NULL;
    }

    ESP_LOGI(TAG, "Http Server started on port: [%d]", config.server_port);

    httpd_uri_t get_dataset = {.uri = "/api/dataset",
                               .method = HTTP_GET,
                               .handler = get_dataset_handler,
                               .user_ctx = server_wrapper->ctx};

    httpd_uri_t switch_state = {.uri = "/api/switch-state",
                                .method = HTTP_POST,
                                .handler = post_switch_state_handler,
                                .user_ctx = server_wrapper->ctx};

    httpd_uri_t reset_dataset = {.uri = "/api/reset",
                                 .method = HTTP_POST,
                                 .handler = post_reset_dataset_handler,
                                 .user_ctx = server_wrapper->ctx};

    httpd_uri_t get_map = {.uri = "/api/map",
                           .method = HTTP_GET,
                           .handler = get_map_handler,
                           .user_ctx = server_wrapper->ctx};

    httpd_uri_t get_state = {.uri = "/api/state",
                             .method = HTTP_GET,
                             .handler = get_state_handler,
                             .user_ctx = server_wrapper->ctx};

    httpd_uri_t get_static = {.uri = "/*",
                              .method = HTTP_GET,
                              .handler = static_file_handler,
                              .user_ctx = NULL};

    httpd_register_uri_handler(server_wrapper->server, &get_dataset);
    httpd_register_uri_handler(server_wrapper->server, &get_map);
    httpd_register_uri_handler(server_wrapper->server, &switch_state);
    httpd_register_uri_handler(server_wrapper->server, &reset_dataset);
    httpd_register_uri_handler(server_wrapper->server, &get_state);
    httpd_register_uri_handler(server_wrapper->server, &get_static);

    return server_wrapper;
}

void http_server_stop(ServerWrapper *server_wrapper)
{
    ESP_LOGI(TAG, "Stopping Http Server...");

    if (!server_wrapper)
        return;

    if (server_wrapper->server) {
        httpd_stop(server_wrapper->server);
    }

    free(server_wrapper->ctx);
    free(server_wrapper);

    ESP_LOGI(TAG, "Http Server stopped");
}
