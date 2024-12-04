#pragma once

#include "esp_http_server.h"

#define HTTP_SERVER_PORT 80
#define HTTP_SERVER_MAX_BUFFER_SIZE 512

int on_get_sync(char* path, esp_err_t(*get_handler) (httpd_req_t* req));

httpd_handle_t on_get_async(char* path, esp_err_t(*get_handler) (httpd_req_t* req));
