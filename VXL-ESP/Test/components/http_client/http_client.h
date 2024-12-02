#pragma once
#include "esp_http_client.h"

#define MAX_HTTP_OUTPUT_BUFFER 512

int http_request(char* url, esp_http_client_method_t http_method, char* req, char* res);