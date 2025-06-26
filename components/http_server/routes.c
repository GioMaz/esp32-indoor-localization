#include "routes.h"
#include "dataset.h"
#include "esp_http_server.h"
#include "http_server.h"
#include "storage.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

esp_err_t get_state_handler(httpd_req_t *req)
{
    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

    State *state = ctx->state;
    const char *state_str = (*state == STATE_TRAINING) ? "training" : "inference";

    char json_response[32];
    snprintf(json_response, sizeof(json_response), "{\"state\":\"%s\"}", state_str);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t post_switch_state_handler(httpd_req_t *req)
{
    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

    State *state = ctx->state;
    if (!toggle_state(state)) {
        return ESP_FAIL;
    }

    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

esp_err_t post_reset_dataset_handler(httpd_req_t *req)
{
    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

    dataset_init(ctx->dataset);

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t get_map_handler(httpd_req_t *req)
{
    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");

    char json_response[64];
    httpd_resp_send_chunk(req, "{", 1);

    int data_count = ctx->dataset->data_count;

    {
        Pos pos = ctx->position;
        int x = round(pos.x);
        int y = round(pos.y);
        int len = snprintf(json_response, sizeof(json_response),
                           "\"position\": {\"x\": %d, \"y\": %d}%s", x, y,
                           (data_count > 0) ? "," : "");

        if (httpd_resp_send_chunk(req, json_response, len) != ESP_OK) {
            httpd_resp_sendstr_chunk(req, NULL);
            return ESP_FAIL;
        }
    }

    if (data_count > 0) {
        const char *prefix = "\"dataset\": [";
        httpd_resp_send_chunk(req, prefix, strlen(prefix));

        for (int i = 0; i < data_count; i++) {
            Pos pos = ctx->dataset->data[i].pos;
            int x = round(pos.x);
            int y = round(pos.y);
            int len =
                snprintf(json_response, sizeof(json_response), "{\"x\": %d, \"y\": %d}%s",
                         x, y, i < data_count - 1 ? "," : "");

            if (httpd_resp_send_chunk(req, json_response, len) != ESP_OK) {
                httpd_resp_sendstr_chunk(req, NULL);
                return ESP_FAIL;
            }
        }

        httpd_resp_send_chunk(req, "]", 1);
    }

    httpd_resp_send_chunk(req, "}", 1);
    httpd_resp_send_chunk(req, NULL, 0); // transmission end

    return ESP_OK;
}

esp_err_t get_position_handler(httpd_req_t *req)
{
    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

    int x = round(ctx->position.x);
    int y = round(ctx->position.y);

    char json_response[64];
    snprintf(json_response, sizeof(json_response), "{\"x\": %d, \"y\": %d}", x, y);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_dataset_handler(httpd_req_t *req)
{

    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx || !ctx->dataset) {
        return ESP_FAIL;
    }

    // Set headers for binary transfer
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Content-Disposition",
                       "attachment; filename=\"dataset.bin\"");

    printf("dataset size %ld\n", ctx->dataset->data_count);

    // End chunked transfer
    httpd_resp_send(req, (const char *)ctx->dataset, sizeof(Dataset));

    return ESP_OK;
}

const char *get_content_type(const char *filepath)
{
    const char *ext = strrchr(filepath, '.');
    if (!ext)
        return "text/plain";

    if (strcmp(ext, ".html") == 0)
        return "text/html";
    if (strcmp(ext, ".htm") == 0)
        return "text/html";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".js") == 0)
        return "application/javascript";
    if (strcmp(ext, ".json") == 0)
        return "application/json";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".jpg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".gif") == 0)
        return "image/gif";
    if (strcmp(ext, ".svg") == 0)
        return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(ext, ".txt") == 0)
        return "text/plain";
    if (strcmp(ext, ".pdf") == 0)
        return "application/pdf";

    return "application/octet-stream";
}

esp_err_t static_file_handler(httpd_req_t *req)
{
    char filepath[PATH_LEN] = {0};

    // If URI is "/", map to /index.html
    if (strcmp(req->uri, "/") == 0) {
        snprintf(filepath, sizeof(filepath), "%s/index.html", BASE_PATH);
    } else {
        snprintf(filepath, sizeof(filepath), "%s%s", BASE_PATH, req->uri);
    }

    FILE *file = fopen(filepath, "r");
    if (!file) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, get_content_type(filepath));

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (httpd_resp_send_chunk(req, buffer, read_bytes) != ESP_OK) {
            fclose(file);
            httpd_resp_sendstr_chunk(req, NULL);
            return ESP_FAIL;
        }
    }

    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0); // transmission end
    return ESP_OK;
}
