# ESP32 controller medium
Project used for a ESP32 device that behaves as a medium between motor controller and a broker server.

## Installation prerequisite
ESP-IDF V5.0 or newer is recommended.

## Components usage

### HTTP client
 - Import
 ``` c
 #include "http_client.h"
 ```
 - Method:
    ``` c
    int http_request(
        char* url,
        esp_http_client_method_t http_method,
        char* req_JSON,
        char* res
    );
    ```

    Params explanation:

    `url`: full url path including protocol, host, port, path, and queries.

    `http_method`: choose between `HTTP_GET`, `HTTP_POST`, `HTTP_PATCH`.

    `req_JSON`: request body buffer (should be in JSON format). Only applicable when `http_method` is `HTTP_POST`. Can be NULL if ignore request body.

    `res`: response body buffer. Can be NULL if ignore response.

    Return value: 0 means success, fail when value is not 0.

### HTTPS client
Coming soon

### HTTP server
 - Import
    ``` c
    #include "http_server.h"
    ```
 - Methods:

    ``` c
    httpd_handle_t on_get_async(
        const char* path,
        esp_err_t(*handler) (httpd_req_t* req)
    );

    httpd_handle_t on_post_async(
        const char* path,
        esp_err_t(*handler) (httpd_req_t* req)
    );
    ```
    Register a path, method, and callback to server.

    Params explanation:

    `path`:
    a path for server listen to, must start with `/`.
    E.g., `/example`

    `handler`: a callback to process data when client connect to the corresponding path.

    Please check out this example: https://github.com/espressif/esp-idf/blob/master/examples/protocols/http_server/simple/main/main.c to learn more about how to create a callback.

### Wi-Fi connection

### test_compo

### Rotary Encoder
Please refer to: https://github.com/nopnop2002/esp-idf-RotaryEncoder
