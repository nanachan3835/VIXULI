// /*
//  * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
//  *
//  * SPDX-License-Identifier: CC0-1.0
//  */

// #include "sdkconfig.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "esp_log.h"
// #include "driver/pulse_cnt.h"
// #include "driver/gpio.h"
// #include "esp_sleep.h"

// static const char *TAG = "example";

// #define EXAMPLE_PCNT_HIGH_LIMIT 100
// #define EXAMPLE_PCNT_LOW_LIMIT  -100

// #define EXAMPLE_EC11_GPIO_A 0
// #define EXAMPLE_EC11_GPIO_B 2

// static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
// {
//     BaseType_t high_task_wakeup;
//     QueueHandle_t queue = (QueueHandle_t)user_ctx;
//     // send event data to queue, from this interrupt callback
//     xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
//     return (high_task_wakeup == pdTRUE);
// }

// void app_main(void)
// {
//     ESP_LOGI(TAG, "install pcnt unit");
//     pcnt_unit_config_t unit_config = {
//         .high_limit = EXAMPLE_PCNT_HIGH_LIMIT,
//         .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
//     };
//     pcnt_unit_handle_t pcnt_unit = NULL;
//     ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

//     ESP_LOGI(TAG, "set glitch filter");
//     pcnt_glitch_filter_config_t filter_config = {
//         .max_glitch_ns = 1000,
//     };
//     ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

//     ESP_LOGI(TAG, "install pcnt channels");
//     pcnt_chan_config_t chan_a_config = {
//         .edge_gpio_num = EXAMPLE_EC11_GPIO_A,
//         .level_gpio_num = EXAMPLE_EC11_GPIO_B,
//     };
//     pcnt_channel_handle_t pcnt_chan_a = NULL;
//     ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
//     pcnt_chan_config_t chan_b_config = {
//         .edge_gpio_num = EXAMPLE_EC11_GPIO_B,
//         .level_gpio_num = EXAMPLE_EC11_GPIO_A,
//     };
//     pcnt_channel_handle_t pcnt_chan_b = NULL;
//     ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

//     ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
//     ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
//     ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
//     ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
//     ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

//     ESP_LOGI(TAG, "add watch points and register callbacks");
//     int watch_points[] = {EXAMPLE_PCNT_LOW_LIMIT, -50, 0, 50, EXAMPLE_PCNT_HIGH_LIMIT};
//     for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
//         ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
//     }
//     pcnt_event_callbacks_t cbs = {
//         .on_reach = example_pcnt_on_reach,
//     };
//     QueueHandle_t queue = xQueueCreate(10, sizeof(int));
//     ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

//     ESP_LOGI(TAG, "enable pcnt unit");
//     ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
//     ESP_LOGI(TAG, "clear pcnt unit");
//     ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
//     ESP_LOGI(TAG, "start pcnt unit");
//     ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

// #if CONFIG_EXAMPLE_WAKE_UP_LIGHT_SLEEP
//     // EC11 channel output high level in normal state, so we set "low level" to wake up the chip
//     ESP_ERROR_CHECK(gpio_wakeup_enable(EXAMPLE_EC11_GPIO_A, GPIO_INTR_LOW_LEVEL));
//     ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());
//     ESP_ERROR_CHECK(esp_light_sleep_start());
// #endif

//     // Report counter value
//     int pulse_count = 0;
//     int event_count = 0;
//     while (1) {
//         if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(1000))) {
//             ESP_LOGI(TAG, "Watch point event, count: %d", event_count);
//         } else {
//             ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
//             ESP_LOGI(TAG, "Pulse count: %d", pulse_count);
//         }
//     }
// }


#include <stdio.h>
#include <inttypes.h> // For PRIu32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

// ADC Configuration
#define ADC_CHANNEL    ADC1_CHANNEL_0 // GPIO36 (VP) by default
#define ADC_WIDTH      ADC_WIDTH_BIT_12
#define ADC_ATTEN      ADC_ATTEN_DB_11
#define DEFAULT_VREF   1100 // Default VREF in mV (set according to your ESP32 board)

static esp_adc_cal_characteristics_t *adc_chars;

void app_main(void)
{
    // Allocate memory for ADC characteristics
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));

    // Configure ADC width and attenuation
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars);

    printf("ADC configured. Reading values...\n");

    while (1) {
        // Read raw value from ADC
        int raw_value = adc1_get_raw(ADC_CHANNEL);

        // Convert raw value to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);

        // Print raw and voltage values
        printf("Raw: %d\tVoltage: %" PRIu32 "mV\n", raw_value, voltage);

        // Delay for readability
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
