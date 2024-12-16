#include "http_server.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>

#define TAG "http_server"

static httpd_handle_t server = NULL;

httpd_handle_t on_get_async(const char* path, esp_err_t(*handler) (httpd_req_t* req) ) {
    if (path == NULL || strncmp(path, "/", 1) != 0) {
        ESP_LOGE(TAG, "Invalid path: %.*s", (int)strlen(path), path);
        return NULL;
    }
    httpd_uri_t uri_get = {
        .uri      = path,
        .method   = HTTP_GET,
        .handler  = handler,
        .user_ctx = NULL
    };
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (server) {
        httpd_register_uri_handler(server, &uri_get);
    }
    else if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

httpd_handle_t on_post_async(const char* path, esp_err_t(*handler) (httpd_req_t* req) ) {
    if (path == NULL || strncmp(path, "/", 1) != 0) {
        ESP_LOGE(TAG, "Invalid path: %.*s", (int)strlen(path), path);
        return NULL;
    }
    httpd_uri_t uri_get = {
        .uri      = path,
        .method   = HTTP_POST,
        .handler  = handler,
        .user_ctx = NULL
    };
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (server) {
        httpd_register_uri_handler(server, &uri_get);
    }
    else if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}