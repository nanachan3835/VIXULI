#include "wifi_connect.h"

#include <inttypes.h>
#include <string.h>
#include "esp_sntp.h"
#include "freertos/event_groups.h"
#include "stdbool.h"

#define WIFI_AUTHMODE WIFI_AUTH_WPA2_PSK

#define TAG "wifi_connect"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static esp_netif_t *wifi_netif = NULL;
static esp_event_handler_instance_t ip_event_handler;
static esp_event_handler_instance_t wifi_event_handler;

static EventGroupHandle_t s_wifi_event_group = NULL;

static bool is_connect = false;
static bool is_init = false;

static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Handling IP event, event code 0x%" PRIx32, event_id);
    switch (event_id)
    {
    case (IP_EVENT_STA_GOT_IP):
        ip_event_got_ip_t *event_ip = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event_ip->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case (IP_EVENT_STA_LOST_IP):
        ESP_LOGI(TAG, "Lost IP");
        wifi_disconnect();
        break;
    case (IP_EVENT_GOT_IP6):
        ip_event_got_ip6_t *event_ip6 = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(TAG, "Got IPv6: " IPV6STR, IPV62STR(event_ip6->ip6_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        ESP_LOGI(TAG, "IP event not handled");
        break;
    }
}

static void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Handling Wi-Fi event, event code 0x%" PRIx32, event_id);

    switch (event_id)
    {
    case (WIFI_EVENT_WIFI_READY):
        ESP_LOGI(TAG, "Wi-Fi ready");
        break;
    case (WIFI_EVENT_SCAN_DONE):
        ESP_LOGI(TAG, "Wi-Fi scan done");
        break;
    case (WIFI_EVENT_STA_START):
        ESP_LOGI(TAG, "Wi-Fi started, connecting to AP...");
        esp_wifi_connect();
        break;
    case (WIFI_EVENT_STA_STOP):
        ESP_LOGI(TAG, "Wi-Fi stopped");
        break;
    case (WIFI_EVENT_STA_CONNECTED):
        ESP_LOGI(TAG, "Wi-Fi connected");
        break;
    case (WIFI_EVENT_STA_DISCONNECTED):
        ESP_LOGI(TAG, "Wi-Fi disconnected");
        ESP_LOGI(TAG, "Retrying to connect to Wi-Fi network...");
        esp_wifi_connect();
        break;
    case (WIFI_EVENT_STA_AUTHMODE_CHANGE):
        ESP_LOGI(TAG, "Wi-Fi authmode changed");
        break;
    default:
        ESP_LOGI(TAG, "Wi-Fi event not handled");
        break;
    }
}

esp_err_t wifi_init(char* static_ip, char* gateway, char* netmask)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    s_wifi_event_group = xEventGroupCreate();

    ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize TCP/IP network stack");
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create default event loop");
        return ret;
    }

    ret = esp_wifi_set_default_wifi_sta_handlers();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set default handlers");
        return ret;
    }

    wifi_netif = esp_netif_create_default_wifi_sta();
    if (wifi_netif == NULL)
    {
        ESP_LOGE(TAG, "Failed to create default WiFi STA interface");
        return ESP_FAIL;
    }
    // Disable DHCP client
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(wifi_netif));
    // Set static IP
    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr = esp_ip4addr_aton(static_ip);
    ip_info.gw.addr = esp_ip4addr_aton(gateway);
    ip_info.netmask.addr = esp_ip4addr_aton(netmask);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(wifi_netif, &ip_info));

    // Wi-Fi stack configuration parameters
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_cb,
                                                        NULL,
                                                        &wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &ip_event_cb,
                                                        NULL,
                                                        &ip_event_handler));

    is_init = true;

    return ret;
}

esp_err_t wifi_init_dhcp() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    s_wifi_event_group = xEventGroupCreate();

    ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize TCP/IP network stack");
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create default event loop");
        return ret;
    }

    ret = esp_wifi_set_default_wifi_sta_handlers();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set default handlers");
        return ret;
    }

    wifi_netif = esp_netif_create_default_wifi_sta();
    if (wifi_netif == NULL)
    {
        ESP_LOGE(TAG, "Failed to create default WiFi STA interface");
        return ESP_FAIL;
    }

    // Wi-Fi stack configuration parameters
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_cb,
                                                        NULL,
                                                        &wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &ip_event_cb,
                                                        NULL,
                                                        &ip_event_handler));

    is_init = true;
    return ret;
}

esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password)
{
    if (is_init == 0)
    {
        ESP_LOGE(TAG, "Need to initialize wifi before connect");
        return ESP_ERR_INVALID_STATE;
    }

    wifi_config_t wifi_config = {
        .sta = {},
    };

    if (wifi_ssid == NULL || wifi_password == NULL)
    {
        ESP_LOGE(TAG, "Wi-Fi SSID or password is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    strncpy((char *)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, wifi_password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "Connecting to Wi-Fi network: %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected to Wi-Fi network: %s", wifi_config.sta.ssid);
        is_connect = true;
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network: %s", wifi_config.sta.ssid);
        return ESP_FAIL;
    }

    ESP_LOGE(TAG, "Unexpected Wi-Fi error");
    return ESP_FAIL;
}

void init_sntp(const char* server) {
    esp_sntp_stop();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, server);
    esp_sntp_init();
}

esp_err_t sync_time() {
    if (!is_connect)
    {
        ESP_LOGE(TAG, "Wi-Fi not connected to sync time");
        return ESP_ERR_INVALID_STATE;
    }

    const char* serverList[] = {
            "time.google.com",
            "time.android.com",
            "time.aws.com",
            "time.cloudflare.com",
            "time.windows.com",
            "time.nist.gov",
            "time.apple.com",
            "time.facebook.com",
            "pool.ntp.org",
            "0.pool.ntp.org",
            "3.pool.ntp.org",
    };

    int retry = 0;
    const int retry_count = 10;
    for (int i = 0; i < sizeof(serverList) / sizeof(const char*); i++) {
        init_sntp(serverList[i]);
        ESP_LOGI(TAG, "Selecting time server: %s", serverList[i]);
        while (++retry < retry_count) {
            if (sntp_get_sync_status() != SNTP_SYNC_STATUS_RESET)
                return ESP_OK;
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        ESP_LOGE(TAG, "Cannot get time from server: %s", serverList[i]);
        retry = 0;
    }

    return ESP_FAIL;
}

esp_err_t wifi_disconnect()
{
    if (s_wifi_event_group)
    {
        vEventGroupDelete(s_wifi_event_group);
    }

    s_wifi_event_group = NULL;
    is_connect = false;

    return esp_wifi_disconnect();
}

esp_err_t wifi_free()
{
    esp_err_t ret = esp_wifi_stop();
    if (ret == ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(TAG, "Wi-Fi stack not initialized");
        return ret;
    }

    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, ip_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler));

    wifi_netif = NULL;
    ip_event_handler = NULL;
    wifi_event_handler = NULL;
    is_init = false;
    return ESP_OK;
}