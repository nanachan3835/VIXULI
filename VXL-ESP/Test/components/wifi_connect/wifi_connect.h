#pragma once

#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_init(const char* static_ip, const char* gateway, const char* netmask);
esp_err_t wifi_init_dhcp();
esp_err_t wifi_connect(char* wifi_ssid, char* wifi_password);
esp_err_t sync_time();
esp_err_t wifi_disconnect(void);
esp_err_t wifi_free(void);

#ifdef __cplusplus
}
#endif