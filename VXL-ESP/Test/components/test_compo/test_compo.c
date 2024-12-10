#include "test_compo.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "wifi_connect.h"
#include <string.h>
#include "driver/gpio.h"
#include "cJSON.h"
#include "time.h"
#include "http_client.h"

#define TAG "test_compo"

float get_speed(int gpio_num1, int gpio_num2) {
    return -1;
}

esp_err_t post_handler (httpd_req_t* req) {
    return ESP_FAIL;
}

int test(char* json) {
    return -1;
}

void event_wifi_handler(char* wifi_ssid, char* wifi_password, int gpio_num1, int gpio_num2) {
    wifi_init_dhcp();
    esp_err_t ret = wifi_connect(wifi_ssid, wifi_password);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
        return;
    }

    esp_err_t err = sync_time();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to sync time");
        return;
    }

    cJSON *root = cJSON_CreateObject();

    float speed = get_speed(gpio_num1, gpio_num2);
    char speed_str[16];
    snprintf(speed_str, sizeof(speed_str), "%f", speed);
    time_t now;
    time(&now);

    cJSON_AddStringToObject(root, "speed", speed_str);
    cJSON_AddNumberToObject(root, "start_time", now);
    cJSON_AddNumberToObject(root, "stop_time", now);

    char *json_string = cJSON_Print(root);
    ESP_LOGI(TAG, "Generated JSON:\n%.*s\n", (int) strlen(json_string) , json_string);

    char res[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int error = -1;
    while (error != 0) {
        ESP_LOGE(TAG, "Failed to send HTTP request");
        error = http_request("http://127.0.0.1/testjson", HTTP_METHOD_POST, json_string, res);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    ESP_LOGI(TAG, "Response:\n%s\n", res);

    // Clean up
    cJSON_Delete(root);
    cJSON_free(json_string);
    wifi_disconnect();
    wifi_free();
}


