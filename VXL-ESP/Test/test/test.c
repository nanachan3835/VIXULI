#ifdef TESTING

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "http_client.h"
#include "wifi_connect.h"

#define TAG "main"

#ifndef WIFI_SSID
#define WIFI_CONFIG_NOT_DEFINED
#endif
#ifndef WIFI_PASSWORD
#define WIFI_CONFIG_NOT_DEFINED
#endif

#ifndef HOST
#define HOST "192.168.1.250"
#endif

#ifndef PORT
#define PORT "8000"
#endif

void app_main(void) {
#ifdef WIFI_CONFIG_NOT_DEFINED
  ESP_LOGE(TAG, "Wifi SSID and password are not defined!");
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(100000));
  }
#else
  ESP_LOGI(TAG, "Starting ...");
  ESP_ERROR_CHECK(wifi_init());

  esp_err_t ret = wifi_connect(WIFI_SSID, WIFI_PASSWORD);
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

    for (int i = 0; i < 6; i++) {
      int error =
          http_request("http://" HOST ":" PORT "/inc", HTTP_METHOD_PATCH, NULL, NULL);
      if (error) {
        ESP_LOGE(TAG, "Failed to send HTTP request");
        i--;
      } else {
        ESP_LOGI(TAG, "HTTP request sent");
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    char res[MAX_HTTP_OUTPUT_BUFFER];
    int http_error =
        http_request("http://" HOST ":" PORT "/getcount", HTTP_METHOD_GET, NULL, res);

    while (http_error) {
      ESP_LOGE(TAG, "Failed to send HTTP request");
      http_error =
          http_request("http://" HOST ":" PORT "/getcount", HTTP_METHOD_GET, NULL, res);
    }

    ESP_LOGI(TAG, "HTTP request sent OK");
    ESP_LOGI(TAG, "Response data: %s", res);
  }

  ESP_ERROR_CHECK(wifi_disconnect());

  ESP_ERROR_CHECK(wifi_free());

  ESP_LOGI(TAG, "End...");

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(100000));
  }
#endif
}

#endif