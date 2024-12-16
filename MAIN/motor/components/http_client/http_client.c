#include "http_client.h"

#include "sys/param.h"

#include "esp_log.h"

#include "esp_http_client.h"


#define TAG "HTTP_CLIENT"

esp_err_t on_http_event(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, response data: %.*s", evt->data_len, (char *)evt->data);
        if (evt->user_data)
        {
            memcpy(evt->user_data, evt->data, MIN(evt->data_len, MAX_HTTP_OUTPUT_BUFFER));
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

int url_validate(char *url)
{
    if (url == NULL)
    {
        return -1;
    }
    if (strlen(url) > 256)
    {
        return -1;
    }
    if (strncmp(url, "http://", 7) != 0)
    {
        return -1;
    }
    return 0;

}

// !: res can be NULL
int http_request(char *url, esp_http_client_method_t http_method, char *req_JSON, char *res)
{
    if (url_validate(url) != 0)
    {
        ESP_LOGE(TAG, "Invalid URL");
        return -1;
    }

    int err_code = 0;

    if (res != NULL) memset(res, 0, MAX_HTTP_OUTPUT_BUFFER); // Clear response buffer

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = on_http_event,
        .user_data = res,
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, http_method);
    if (http_method == HTTP_METHOD_POST && req_JSON != NULL)
    {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, req_JSON, strlen(req_JSON));
    }
    ESP_LOGI(TAG, "sending to url: %s", config.url);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        err_code = -1;
    }

    esp_http_client_cleanup(client);
    return err_code;
}