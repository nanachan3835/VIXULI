#include "test_compo.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "http_server.h"
#include "wifi_connect.h"
#include <cerrno>
#include <string.h>
#include "cJSON.h"
#include "time.h"
#include "http_client.h"

#define TAG "test_compo"

float get_speed(int gpio_num1, int gpio_num2) {
    return -1;
}

esp_err_t post_handler (httpd_req_t* req) { //! untested function
    char* buf = malloc(req->content_len + 1);
    if (buf == NULL) {
        ESP_LOGW(TAG, "Failed to allocate memory for request data. Sending back status 500 to client.");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0) {
        free(buf);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[req->content_len] = '\0';

    ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
    ESP_LOGI(TAG, "%s", buf);
    ESP_LOGI(TAG, "====================================");

    int is_data_valid = test(buf);
    if (is_data_valid == -1) {
        free(buf);
        ESP_LOGW(TAG, "Client sent invalid data");
        httpd_resp_set_status(req, "HTTP/1.1 400 Bad Request");
        httpd_resp_send(req, "Invalid data", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    free(buf);
    httpd_resp_set_status(req, "HTTP/1.1 204 No Content");
    httpd_resp_send(req, NULL, 0); // todo: need to test if it crashes or not
    return ESP_OK;
}

int test(char* json) {
    cJSON *testJson = cJSON_Parse(json);
    ESP_LOGI(TAG, "Parsing JSON:\n %s", json);
    if (testJson == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON.");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "Error before: %s", error_ptr);
        }
        cJSON_Delete(testJson);
        return -1;
    }

    // Parse and validate speed
    char* speed_str = cJSON_GetObjectItem(testJson, "speed")->valuestring;
    if (speed_str == NULL) {
        ESP_LOGE(TAG, "Failed to get speed from JSON");
        cJSON_Delete(testJson);
        return -1;
    }
    void* endptr;
    float speed = strtof(speed_str, endptr);
    errno = 0;
    if (speed_str == endptr) {
        ESP_LOGE(TAG, "Failed to convert speed to float");
        cJSON_Delete(testJson);
        return -1;
    }
    if (errno == ERANGE) {
        ESP_LOGE(TAG, "Speed out of range");
        cJSON_Delete(testJson);
        return -1;
    }
    if (speed < 0) {
        ESP_LOGE(TAG, "Invalid speed value");
        cJSON_Delete(testJson);
        return -1;
    }

    const int GMT7_D1_M12_Y2024_EPOCH = 1732986000;

    // Parse and validate start_time
    int start_time = cJSON_GetObjectItem(testJson, "start_time")->valueint;
    if (start_time == 0) {
        ESP_LOGE(TAG, "Failed to get start_time from JSON");
        cJSON_Delete(testJson);
        return -1;
    }
    if (start_time < GMT7_D1_M12_Y2024_EPOCH) {
        ESP_LOGE(TAG, "Absurd start_time value");
        cJSON_Delete(testJson);
        return -1;
    }

    // Parse and validate stop_time
    int stop_time = cJSON_GetObjectItem(testJson, "stop_time")->valueint;
    if (stop_time == 0) {
        ESP_LOGE(TAG, "Failed to get stop_time from JSON");
        cJSON_Delete(testJson);
        return -1;
    }
    if (stop_time < GMT7_D1_M12_Y2024_EPOCH) {
        ESP_LOGE(TAG, "Absurd stop_time value");
        cJSON_Delete(testJson);
        return -1;
    }

    cJSON_Delete(testJson);
    return 0;
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

    httpd_handle_t server = on_post_async("/testjson", post_handler);
    if (server == NULL) {
        ESP_LOGE(TAG, "Failed to create server");
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // delay to make sure server is on

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


