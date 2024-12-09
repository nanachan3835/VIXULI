#include "test_compo.h"
#include "esp_log.h"
#include "wifi_connect.h"
#include <string.h>
#include "driver/gpio.h"
#include "cJSON.h"
#include "time.h"
#include "http_client.h"
#include <stdlib.h>

#define TAG "test_compo"

int test(char* json) {
    return 0;
}

void event_wifi_handler(char* wifi_ssid, char* wifi_password) {
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

    int speed = gpio_get_level(2); // todo: ask Dung about the pin number
    time_t now;
    time(&now);

    cJSON_AddNumberToObject(root, "speed", speed);
    cJSON_AddNumberToObject(root, "start_time", now);
    cJSON_AddNumberToObject(root, "stop_time", now);

    char *json_string = cJSON_Print(root);
    ESP_LOGI(TAG, "Generated JSON:\n%.*s\n", (int) strlen(json_string) , json_string);

    char *res = (char *) malloc(MAX_HTTP_OUTPUT_BUFFER);
    int error = http_request("http://127.0.0.1/testjson", HTTP_METHOD_POST, json_string, res);
    if (error != 0) {
        ESP_LOGE(TAG, "Failed to send HTTP request");
        return;
    }
    ESP_LOGI(TAG, "Response:\n%s\n", res);

    // Clean up
    cJSON_Delete(root);
    cJSON_free(json_string);
    free(res);
}


