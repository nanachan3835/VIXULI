#include <stdio.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

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




void app_main(void) {
    //const uart_port_t uart_num = UART_NUM_2;
    //khoi tao uart
    uart_init();
    
    uint8_t data[BUF_SIZE];

    while (1) {
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            printf("Read %d bytes: '%.*s'\n", len, len, data);
            uart_write_bytes(uart_num, (const char*)data, len);
            uart_flush(uart_num);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
