#include <stdio.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "http_server.h"
#include "http_client.h"
#include "wifi_connect.h"

#define WIFI_SSID "Wifi"
#define WIFI_PASSWORD "nduc2003"
#define SERVER_HOST "192.168.190.91"
#define SERVER_PORT "8000"
#define STATIC_IP "192.168.1.240"
#define GATEWAY "192.168.1.1"
#define NETMASK "255.255.255.0"

#define TAG "main"
#define BUF_SIZE 128
const uart_port_t uart_num = UART_NUM_2;



void uart_init()
{
     uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 4, 5, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
}




// void app_main(void) {
//     //const uart_port_t uart_num = UART_NUM_2;
//     //khoi tao uart
//     uart_init();
    
//     uint8_t data[BUF_SIZE];

//     while (1) {
//         int len = uart_read_bytes(uart_num, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        // if (len > 0) {
        //     printf("Read %d bytes: '%.*s'\n", len, len, data);
        //     uart_write_bytes(uart_num, (const char*)data, len);
        //     uart_flush(uart_num);
        // }

//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

esp_err_t test_get_handler(httpd_req_t *req)
{
    const char *resp_str = "Hello, World!";
    httpd_resp_send(req, req->user_ctx, strlen(req->user_ctx));
    return ESP_OK;
}


void app_main()
{   ESP_ERROR_CHECK(wifi_init_dhcp());
    uart_init();
    char res_data[BUF_SIZE];
    //event_wifi_handler(WIFI_SSID, WIFI_PASSWORD, 0, 1);
    httpd_handle_t server = on_get_with_data_async("/test",(char*)res_data, test_get_handler);
    esp_err_t ret = wifi_connect(WIFI_SSID, WIFI_PASSWORD);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
        }   
    while (1) {
        //printf("Waiting for data...");
        uint8_t data[BUF_SIZE];
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            printf("Read %d bytes: '%.*s'\n", len, len, data);
            uart_write_bytes(uart_num, (const char*)data, len);

        
        ESP_LOGI("HTTP_CLIENT", "Sending JSON: %s", data);
        strcpy(res_data, (char*)data);
        res_data[BUF_SIZE - 1] = 0;

     
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_ERROR_CHECK(wifi_disconnect());
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_ERROR_CHECK(wifi_free());
        uart_flush(uart_num);

        }
        else 
        {
            strcpy(res_data, "no data");
        res_data[BUF_SIZE - 1] = 0;

     
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_ERROR_CHECK(wifi_disconnect());
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_ERROR_CHECK(wifi_free());
        }

    //     else
    //     {
    //         esp_err_t ret = wifi_connect(WIFI_SSID, WIFI_PASSWORD);
    //     if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
    // }   
    //     ESP_LOGI("HTTP_CLIENT", "Sending JSON: NULL");

    //     // int http_error = http_request("http://" SERVER_HOST ":" SERVER_PORT "/getcount",
    //     //                             HTTP_METHOD_POST,"{\"NULL\":\"No data\"}", NULL);
    //     // if (http_error) {
    //     //     ESP_LOGE(TAG, "Failed to send HTTP request");
    //     // } else {
    //     //     ESP_LOGI(TAG, "HTTP request sent");
    //     // }

    //     // on_get_with_data_async("/test", "no data", test_get_handler);
    //     strcpy((char*)data, "No data");

    //     // while (1)
    //     // {
    //     //     vTaskDelay(pdMS_TO_TICKS(10000));
    //     // }
        
        
    //     vTaskDelay(pdMS_TO_TICKS(10000));
    //     ESP_ERROR_CHECK(wifi_disconnect());
    //     vTaskDelay(pdMS_TO_TICKS(10000));
    //     ESP_ERROR_CHECK(wifi_free());
    //     //uart_flush(uart_num);

    //     disconnect_server(server);
    //     server = NULL;
    //     }


        //ESP_ERROR_CHECK(wifi_connect(WIFI_SSID, WIFI_PASSWORD));
        
    }
            disconnect_server(server);
        server = NULL;
    //ESP_LOGI(TAG, "Starting ...");
    //ESP_ERROR_CHECK(wifi_init(STATIC_IP, GATEWAY, NETMASK));
}
