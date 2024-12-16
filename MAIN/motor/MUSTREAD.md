# ** CAC CHAN CAM VAO ESP **

## MOTOR

- IN1 = CHAN 25
- IN2 = CHAN 26
- ENABLE = CHAN 27

## BUTTON

- BUTTON 1 = CHAN 12
- BUTTON 2 = CHAN 4
- BUTTON 3 = CHAN 5

## NUM XOAY 

- TINHIEU = VP
- "+" = 3V3
- "-" = GND


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

### Motor 

- Methods:

    1. `motor_control_init()`:
        ```c
        void motor_control_init() {
        // Cấu hình Timer cho LEDC
        ledc_timer_config_t ledc_timer = {
                .speed_mode       = LEDC_MODE,
                .duty_resolution  = LEDC_RESOLUTION,
                .timer_num        = LEDC_TIMER,
                .freq_hz          = LEDC_FREQUENCY,
                .clk_cfg          = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        // Cấu hình kênh PWM cho LEDC
         ledc_channel_config_t ledc_channel = {
        .gpio_num       = LEDC_OUTPUT_IO,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // Bắt đầu với duty cycle 0
        .hpoint         = 0
        };  
        ```
    Initialize resource to run motor.

    2. ` motor_set_speed() `
        ```c
        void motor_set_speed(int speed) 
        ``` 
        Set speed for motor

        __Caution:__
         
        The speed is between -255 to 255, if more or less than that, it will set the Maximum |speed| = 255. 

        And the rotate direction of motor depend on the speed is ">" or "<" 0.
    
    __USING ORDER__
    1. First init()
    2. Then you can setting motor speed in any function you want

### BUTTON

- Methods: 
    `configure_button()`:

    ```c
    void configure_button() {
    gpio_set_direction(BUTTON_PIN_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_1, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTTON_PIN_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_2, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTTON_PIN_3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_3, GPIO_PULLUP_ONLY);
    }
    ```
    Init 3 button for project

    

### Potentiometer

- Methods: 
  
  1. Init potentiometer:
  ```c
  // Allocate memory for ADC characteristics
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

    // Configure ADC width and attenuation
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars);
    ```
   2. get value of potentiometer:
   ```c
   int raw_value = adc1_get_raw(ADC_CHANNEL);
   uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);
    ```
    Must put this in while() loop at running function

    The voltage value is to config speed for motor

    3. convert_voltage_to_speed() 
    ```c
    int convert_voltage_to_speed(uint32_t voltage) {
    int speed;
    if (voltage <= 1650) {
        // Map 0 to 1650 mV to -255 to 0
        speed = (voltage * -255) / 1650;
    } else {
        // Map 1650 to 3300 mV to 0 to 255
        speed = ((voltage - 1650) * 255) / 1650;
    }
    return speed;
    }
    ```
    Change voltage to speed to set speed for motor 

    uint32_t for surely set voltage even it take 32bit memory(change later)

# REFERENCE 

- Please refer to: 

   https://github.com/nopnop2002/esp-idf-RotaryEncoder

   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html

   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_wifi.html

   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html

   https://esp32tutorials.com/oled-esp32-esp-idf-tutorial/

   https://github.com/nopnop2002/esp-idf-ssd1306