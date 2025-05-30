# CAC CHAN CAM 
## ESP

- TX = CHAN 4 : Đầu vào TX của UART
- GX = CHAN 5 : Đầu ra GX của UART
- GND  : nối chung với GND của ARDUINO để  đồng bộ tín hiệu

## ARDUINO 

- motorEnable = 9 : Chân PWM điều khiển tốc độ động cơ 
- motorIn1 = 8 : Chân điều khiển chiều động cơ
- motorIn2 = 10 : Chân điều khiển chiều động cơ
- button  = 7,6,5 : Các chân tương ứng với nút bấm 1 2 3 
- potentiometer = A0 : chân analog nhận tín hiệu digital
- Numreadings = 2 : chân đọc giá trị cảm biến quang trở tốc độ

# ESP32 controller medium
Project used for a ESP32 device that acts as a medium between motor controller and a broker server.

## Installation prerequisite
ESP-IDF V5.0 or newer is required.

## Components usage

### HTTP client
 - Import
 ``` c
 #include "http_client.h"
 ```
 - Available function:
    ``` c
    int http_request(
        char* url,
        esp_http_client_method_t http_method,
        char* req_JSON,
        char* res
    );
    ```

    __Parameters:__

    `url`: full url path including protocol, host, port, path, and queries. Example: `http://example.com/api?param1=1&param2=2`

    `http_method`: choose between `HTTP_GET`, `HTTP_POST`, `HTTP_PATCH`.

    `req_JSON`: request body buffer (should be in JSON format). Only applicable when `http_method` is `HTTP_POST`. Can be NULL if ignore request body.

    `res`: response body buffer. Can be NULL if ignore response.

    __Return value:__
     - Returns 0 means success.
     - Any non-zero value means the request failed.

### HTTPS client
Coming soon

### HTTP server
The HTTP server listens for incoming HTTP requests and handles them.
 - Import
    ``` c
    #include "http_server.h"
    ```
 - Functions:

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
    Register a path, method, and callback to server. A server handler will be automatically created when registering for the first time.

    __Parameters:__

    `path`:
    a endpoint path for the server listens on, must start with `/`.
    (E.g., `/example`)

    `handler`: a callback to process data when client connect to the corresponding endpoint path.

    __Example:__

    Please check out this example: https://github.com/espressif/esp-idf/blob/master/examples/protocols/http_server/simple/main/main.c to learn more about how to create a callback.

   __Advance__
   ```c
   httpd_handle_t on_get_with_data_async(
    const char* path, 
    const char* res_data, 
    esp_err_t(*handler) (httpd_req_t* req) 
   );
   ```
   This function register a path, data, and call back to server. We can use this when registering for the first time ( recommend register for one time only before main which loop ).

   The data will automatically update when refresh the web page
   ```c
   void disconnect_server(httpd_handle_t server)
   ```
   After the job done, we need to free the resource so we can use this function.

   PLease use this function after which loop.
### Wi-Fi connection & Time sync
 - Import
    ``` c
    #include "wifi_connect.h"
    ```
 - Methods:

    1. `wifi_init()`:
        ``` c
        esp_err_t wifi_init(char* static_ip, char* gateway, char* netmask);
        ```
        Initialize wifi resources with static IPv4 address.

        Example usage:
        ```c
        ESP_ERROR_CHECK(wifi_init("192.168.1.100", "192.168.1.1", "255.255.255.0"));
        ```
    2. `wifi_init_dhcp()`:
        ```c
        esp_err_t wifi_init_dhcp();
        ```
        Similar to `wifi_init()` but uses DHCP to obtain IP address.

    3. `wifi_connect()`:

        ```c
        esp_err_t wifi_connect(char* wifi_ssid, char* wifi_password);
        ```
        Try to connect to wifi_ssid with given wifi_password. When failed to connect, it will try to reconnect indefinitely until successfully connect.

        Example usage:
        ```c
        ESP_ERROR_CHECK(wifi_connect("wifi_name", "wifipassword"));
        ```
    4. `sync_time()`:
        ```c
        esp_err_t sync_time();
        ```
        Sync time with some predefined SNTP servers. This function will block until successfully get correct time from any server in the server list.

        Example usage:
        ```c
        esp_err_t err = sync_time();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to sync time");
            return;
        }
        ```
    5. `wifi_disconnect`:
        ```c
        esp_err_t wifi_disconnect();
        ```
        Disconnect wifi.
    6. `wifi_free()`:
        ```c
        esp_err_t wifi_free();
        ```
        Free all resources related to wifi connection.

    __Usage Order__

    Please follow this sequence to avoid errors:

    1. Call either `wifi_init()` or `wifi_init_dhcp()`.

    2. Call `wifi_connect()`.

    3. (Optional) Call `sync_time()` to synchronize the clock.

    4. Call `wifi_disconnect()` if needed.

    5. Finally, call `wifi_free()` to release resources.

### test_compo
For testing

### UART 
- Method
1. `uart_init():`
```c
void uart_init()
{
     uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 4, 5, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
}
```

- Initialize uart resource

2. `Data process`
- Use this to create a buffer for store data
```c
uint8_t data[BUF_SIZE];
```
- Use this to get data 
```c
int len = uart_read_bytes(uart_num, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
```
- Use this to free the memory if data is not use
```c
uart_flush(uart_num);
```

# ARDUINO_UNO CONTROLLER 

## Installation and prequesite
- LiquidCrystal_I2C.h lib
- ArduinoJson.h lib 

## CODE 
- please refer to : https://github.com/nanachan3835/VIXULI/blob/main/CODE-ARDUINO-IDE/main/sketch_dec29a/sketch_dec29a.ino for more detail 





# REFERENCE 

- Please refer to: 

   https://github.com/nopnop2002/esp-idf-RotaryEncoder

   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html

   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_wifi.html

   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html

   https://esp32tutorials.com/oled-esp32-esp-idf-tutorial/

   https://github.com/nopnop2002/esp-idf-ssd1306