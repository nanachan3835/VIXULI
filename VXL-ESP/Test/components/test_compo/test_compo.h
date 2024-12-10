#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int test(char* json);
void event_wifi_handler(char* wifi_ssid, char* wifi_password, int gpio_num1, int gpio_num2);

#ifdef __cplusplus
}
#endif