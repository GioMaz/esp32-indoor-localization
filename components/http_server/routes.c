#include "routes.h"
#include "http_server.h"
#include "storage.h"

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

esp_err_t get_dataset_handler(httpd_req_t *req) {

    server_context_t *ctx = (server_context_t *)req->user_ctx;
    if (!ctx) {
        return ESP_FAIL;
    }

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
