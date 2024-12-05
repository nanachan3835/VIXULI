#ifdef TESTING
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "http_client.h"
#include "http_server.h"
#include "wifi_connect.h"

#define TAG "main"

#define WIFI_MAX_RECONNECT_TIMES 30

#define MIN(a, b) a < b ? a : b

esp_err_t get_handler(httpd_req_t *req) {
    const char* base_res = "GET data: ";
    char resp[256] = {0};
    char buf[200];
    int buf_len = MIN(sizeof(buf) - 1, httpd_req_get_url_query_len(req) + 1);

    if (buf_len > 1) {
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            }
    }

    if (sizeof(resp) - strlen(base_res) - buf_len - 1 > 0) {
        strcpy(resp, base_res);
        strcat(resp, buf);
    } else ESP_LOGE(TAG, "Request data too big, data will be ignored.");
    buf[buf_len] = '\0';

    ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
    ESP_LOGI(TAG, "%.*s", buf_len, buf);
    ESP_LOGI(TAG, "====================================");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting ...");
    ESP_ERROR_CHECK(wifi_init(STATIC_IP, GATEWAY, NETMASK));

    esp_err_t ret =
        wifi_connect(WIFI_SSID, WIFI_PASSWORD, WIFI_MAX_RECONNECT_TIMES);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
    }

    wifi_ap_record_t ap_info;
    ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_ERR_WIFI_CONN) {
        ESP_LOGE(TAG, "Wi-Fi station interface not initialized");
    } else if (ret == ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGE(TAG, "Wi-Fi station is not connected");
    } else {
        ESP_LOGI(TAG, "--- Access Point Information ---");
        ESP_LOG_BUFFER_HEX("MAC Address", ap_info.bssid, sizeof(ap_info.bssid));
        ESP_LOG_BUFFER_CHAR("SSID", ap_info.ssid, sizeof(ap_info.ssid));
        ESP_LOGI(TAG, "Primary Channel: %d", ap_info.primary);
        ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);

        ESP_LOGI(TAG, "===========Now test http client==============");

        for (int i = 0; i < 6; i++) {
            int error = http_request("http://" SERVER_HOST ":" SERVER_PORT "/inc",
                                    HTTP_METHOD_PATCH, NULL, NULL);
            if (error) {
                ESP_LOGE(TAG, "Failed to send HTTP request");
                i--;
            } else {
                ESP_LOGI(TAG, "HTTP request sent");
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        char res[MAX_HTTP_OUTPUT_BUFFER];
        int http_error = http_request("http://" SERVER_HOST ":" SERVER_PORT "/getcount",
                                    HTTP_METHOD_GET, NULL, res);

        while (http_error) {
            ESP_LOGE(TAG, "Failed to send HTTP request");
            http_error = http_request("http://" SERVER_HOST ":" SERVER_PORT "/getcount",
                                        HTTP_METHOD_GET, NULL, res);
        }

        ESP_LOGI(TAG, "HTTP request sent OK");
        ESP_LOGI(TAG, "Response data: %s", res);

        ESP_LOGI(TAG, "==============Now test http server==============");

        on_get_sync("/test", get_handler);
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }

    ESP_ERROR_CHECK(wifi_disconnect());

    ESP_ERROR_CHECK(wifi_free());

    ESP_LOGI(TAG, "End...");
    esp_restart();
}

#endif