// // // /*
// // //  * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
// // //  *
// // //  * SPDX-License-Identifier: CC0-1.0
// // //  */

// // // #include "sdkconfig.h"
// // // #include "freertos/FreeRTOS.h"
// // // #include "freertos/task.h"
// // // #include "freertos/queue.h"
// // // #include "esp_log.h"
// // // #include "driver/pulse_cnt.h"
// // // #include "driver/gpio.h"
// // // #include "esp_sleep.h"

// // // static const char *TAG = "example";

// // // #define EXAMPLE_PCNT_HIGH_LIMIT 100
// // // #define EXAMPLE_PCNT_LOW_LIMIT  -100

// // // #define EXAMPLE_EC11_GPIO_A 0
// // // #define EXAMPLE_EC11_GPIO_B 2

// // // static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
// // // {
// // //     BaseType_t high_task_wakeup;
// // //     QueueHandle_t queue = (QueueHandle_t)user_ctx;
// // //     // send event data to queue, from this interrupt callback
// // //     xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
// // //     return (high_task_wakeup == pdTRUE);
// // // }

// // // void app_main(void)
// // // {
// // //     ESP_LOGI(TAG, "install pcnt unit");
// // //     pcnt_unit_config_t unit_config = {
// // //         .high_limit = EXAMPLE_PCNT_HIGH_LIMIT,
// // //         .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
// // //     };
// // //     pcnt_unit_handle_t pcnt_unit = NULL;
// // //     ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

// // //     ESP_LOGI(TAG, "set glitch filter");
// // //     pcnt_glitch_filter_config_t filter_config = {
// // //         .max_glitch_ns = 1000,
// // //     };
// // //     ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

// // //     ESP_LOGI(TAG, "install pcnt channels");
// // //     pcnt_chan_config_t chan_a_config = {
// // //         .edge_gpio_num = EXAMPLE_EC11_GPIO_A,
// // //         .level_gpio_num = EXAMPLE_EC11_GPIO_B,
// // //     };
// // //     pcnt_channel_handle_t pcnt_chan_a = NULL;
// // //     ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
// // //     pcnt_chan_config_t chan_b_config = {
// // //         .edge_gpio_num = EXAMPLE_EC11_GPIO_B,
// // //         .level_gpio_num = EXAMPLE_EC11_GPIO_A,
// // //     };
// // //     pcnt_channel_handle_t pcnt_chan_b = NULL;
// // //     ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

// // //     ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
// // //     ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
// // //     ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
// // //     ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
// // //     ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

// // //     ESP_LOGI(TAG, "add watch points and register callbacks");
// // //     int watch_points[] = {EXAMPLE_PCNT_LOW_LIMIT, -50, 0, 50, EXAMPLE_PCNT_HIGH_LIMIT};
// // //     for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
// // //         ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
// // //     }
// // //     pcnt_event_callbacks_t cbs = {
// // //         .on_reach = example_pcnt_on_reach,
// // //     };
// // //     QueueHandle_t queue = xQueueCreate(10, sizeof(int));
// // //     ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

// // //     ESP_LOGI(TAG, "enable pcnt unit");
// // //     ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
// // //     ESP_LOGI(TAG, "clear pcnt unit");
// // //     ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
// // //     ESP_LOGI(TAG, "start pcnt unit");
// // //     ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

// // // #if CONFIG_EXAMPLE_WAKE_UP_LIGHT_SLEEP
// // //     // EC11 channel output high level in normal state, so we set "low level" to wake up the chip
// // //     ESP_ERROR_CHECK(gpio_wakeup_enable(EXAMPLE_EC11_GPIO_A, GPIO_INTR_LOW_LEVEL));
// // //     ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());
// // //     ESP_ERROR_CHECK(esp_light_sleep_start());
// // // #endif

// // //     // Report counter value
// // //     int pulse_count = 0;
// // //     int event_count = 0;
// // //     while (1) {
// // //         if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(1000))) {
// // //             ESP_LOGI(TAG, "Watch point event, count: %d", event_count);
// // //         } else {
// // //             ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
// // //             ESP_LOGI(TAG, "Pulse count: %d", pulse_count);
// // //         }
// // //     }
// // // }


// // // #include <stdio.h>
// // // #include <inttypes.h> // For PRIu32
// // // #include "freertos/FreeRTOS.h"
// // // #include "freertos/task.h"
// // // #include "driver/adc.h"
// // // #include "esp_adc_cal.h"

// // // // ADC Configuration
// // // #define ADC_CHANNEL    ADC1_CHANNEL_0 // GPIO36 (VP) by default
// // // #define ADC_WIDTH      ADC_WIDTH_BIT_12
// // // #define ADC_ATTEN      ADC_ATTEN_DB_11
// // // #define DEFAULT_VREF   1100 // Default VREF in mV (set according to your ESP32 board)

// // // static esp_adc_cal_characteristics_t *adc_chars;

// // // void app_main(void)
// // // {
// // //     // Allocate memory for ADC characteristics
// // //     adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));

// // //     // Configure ADC width and attenuation
// // //     adc1_config_width(ADC_WIDTH);
// // //     adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

// // //     // Characterize ADC
// // //     esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars);

// // //     printf("ADC configured. Reading values...\n");

// // //     while (1) {
// // //         // Read raw value from ADC
// // //         int raw_value = adc1_get_raw(ADC_CHANNEL);

// // //         // Convert raw value to voltage in mV
// // //         uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);

// // //         // Print raw and voltage values
// // //         printf("Raw: %d\tVoltage: %" PRIu32 "mV\n", raw_value, voltage);

// // //         // Delay for readability
// // //         vTaskDelay(pdMS_TO_TICKS(500));
// // //     }
// // // }




// // #include <driver/i2c.h>
// // #include <esp_log.h>
// // #include <freertos/FreeRTOS.h>
// // #include <freertos/task.h>
// // #include <stdio.h>
// // #include "sdkconfig.h"
// // #include "HD44780.h"
// // #define portTICK_RATE_MS        portTICK_PERIOD_MS
// // #define LCD_ADDR 0x27
// // #define SDA_PIN  21
// // #define SCL_PIN  22
// // #define LCD_COLS 20
// // #define LCD_ROWS 4

// // static char tag[] = "LCD test";
// // void LCD_DemoTask(void* param);

// // void app_main(void)
// // {
// //     ESP_LOGI(tag, "Starting up application");
// //     LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
// //     xTaskCreate(&LCD_DemoTask, "Demo Task", 2048, NULL, 5, NULL);
// // }

// // void LCD_DemoTask(void* param)
// // {
// //     char txtBuf[8];
// //     while (true) {
// //         int row = 0, col = 0;
// //         LCD_home();
// //         LCD_clearScreen();
// //         LCD_writeStr("----- 20x4 LCD -----");
// // }
// // }

// #include <stdbool.h>
// #include <stdint.h>
// #include <stddef.h>
// #include <assert.h>


// int lcdBusIO(lcdDriver_t *driver, bool rw, bool rs, bool en, uint8_t data)
// {
//     // Implement for your own setup.
// }

// int lcdDelay(lcdDriver_t *driver, uint32_t delay)
// {
//     // Implement for your own setup.
//     return usleep((useconds_t)delay);
// }

// void app_main(void)
// {
//     lcdDriver_t lcd = {
//         .userData = NULL,
//         .dimensions = {16, 2},
//         .writeOnly = true,
//         .fourBits = true
//     };

//     // Do GPIO initialization.

//     // Reset GPIO pins to their default state.
//     lcdBusIO(&lcd, false, false, false, 0xFF);

//     lcdLoadDefaultTiming(&lcd);     // Load default LCD timings.
//     lcdInit(&lcd);                  // Initialize the LCD and driver.

//     lcdDirection(&lcd, true);                   // Set the direction to forward.
//     lcdSetDisplay(&lcd, true, false, false);    // Enable display.
//     lcdHome(&lcd);                              // Home the LCD cursor.

//     lcdPutZString(&driver, "Hello World!");
// }

// #include <stdio.h>
// #include "driver/i2c.h"
// #include "esp_log.h"

// #define I2C_MASTER_NUM I2C_NUM_0               // I2C port number for master
// #define I2C_MASTER_SCL_IO 22                   // GPIO number for I2C SCL
// #define I2C_MASTER_SDA_IO 21                   // GPIO number for I2C SDA
// #define I2C_MASTER_FREQ_HZ 100000              // I2C master clock frequency
// #define LCD_ADDR 0x27                          // I2C address of the LCD with PCF8574
// #define WRITE_BIT I2C_MASTER_WRITE             // I2C master write
// #define ACK_CHECK_EN 0x1                       // Enable ACK check
// #define TAG "LCD1602"

// static esp_err_t i2c_master_init() {
//     i2c_config_t conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = I2C_MASTER_SDA_IO,
//         .scl_io_num = I2C_MASTER_SCL_IO,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = I2C_MASTER_FREQ_HZ,
//     };
//     ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
//     ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
//     return ESP_OK;
// }

// static esp_err_t i2c_send_data(uint8_t addr, uint8_t *data, size_t len) {
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     ESP_ERROR_CHECK(i2c_master_start(cmd));
//     ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | WRITE_BIT, ACK_CHECK_EN));
//     ESP_ERROR_CHECK(i2c_master_write(cmd, data, len, ACK_CHECK_EN));
//     ESP_ERROR_CHECK(i2c_master_stop(cmd));
//     esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);
//     return ret;
// }

// // Send a command to the LCD
// static esp_err_t lcd_send_command(uint8_t cmd) {
//     uint8_t upper_nibble = (cmd & 0xF0) | 0x0C; // Enable = 1, RS = 0
//     uint8_t lower_nibble = ((cmd << 4) & 0xF0) | 0x0C;

//     uint8_t data[] = {
//         upper_nibble, (upper_nibble & ~0x04), // Enable = 0
//         lower_nibble, (lower_nibble & ~0x04)  // Enable = 0
//     };

//     return i2c_send_data(LCD_ADDR, data, sizeof(data));
// }

// // Send data to the LCD
// static esp_err_t lcd_send_data(uint8_t data) {
//     uint8_t upper_nibble = (data & 0xF0) | 0x0D; // Enable = 1, RS = 1
//     uint8_t lower_nibble = ((data << 4) & 0xF0) | 0x0D;

//     uint8_t payload[] = {
//         upper_nibble, (upper_nibble & ~0x04), // Enable = 0
//         lower_nibble, (lower_nibble & ~0x04)  // Enable = 0
//     };

//     return i2c_send_data(LCD_ADDR, payload, sizeof(payload));
// }

// // Initialize the LCD
// static esp_err_t lcd_init() {
//     ESP_ERROR_CHECK(lcd_send_command(0x33)); // Initialize in 4-bit mode
//     ESP_ERROR_CHECK(lcd_send_command(0x32)); // Set to 4-bit mode
//     ESP_ERROR_CHECK(lcd_send_command(0x28)); // 2-line, 5x7 matrix
//     ESP_ERROR_CHECK(lcd_send_command(0x0C)); // Display on, cursor off
//     ESP_ERROR_CHECK(lcd_send_command(0x06)); // Increment cursor
//     ESP_ERROR_CHECK(lcd_send_command(0x01)); // Clear display
//     vTaskDelay(5 / portTICK_PERIOD_MS);
//     return ESP_OK;
// }

// // Print a string on the LCD
// static void lcd_print(const char *str) {
//     while (*str) {
//         lcd_send_data((uint8_t)*str++);
//     }
// }

// void app_main() {
//     ESP_LOGI(TAG, "Initializing I2C...");
//     ESP_ERROR_CHECK(i2c_master_init());

//     ESP_LOGI(TAG, "Initializing LCD...");
//     ESP_ERROR_CHECK(lcd_init());
    
//     ESP_LOGI(TAG, "Displaying message...");
//     lcd_print(" NOOOBBBBB");
// }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////test nut bam///////////
// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"

// // Định nghĩa các chân GPIO
// #define BUTTON_1_GPIO GPIO_NUM_15 // Nút 1
// #define BUTTON_2_GPIO GPIO_NUM_4  // Nút 2
// #define BUTTON_3_GPIO GPIO_NUM_5 // Nút 3

// // Hàm khởi tạo GPIO
// void init_buttons() {
//     gpio_config_t button_config = {
//         .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO) | (1ULL << BUTTON_3_GPIO),
//         .mode = GPIO_MODE_INPUT,
//         .pull_up_en = GPIO_PULLUP_ENABLE,  // Bật điện trở kéo lên nội bộ
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE    // Không sử dụng ngắt trong ví dụ này
//     };
//     gpio_config(&button_config);
// }

// // Hàm kiểm tra trạng thái nút bấm
// void check_buttons() {
//     while (1) {
//         if (gpio_get_level(BUTTON_1_GPIO) == 0) {  // Kiểm tra nút 1
//             printf("Button 1 pressed!\n");
//         }
//         else{
//             printf("Button 1 not pressed!\n");
//         }
//         if (gpio_get_level(BUTTON_2_GPIO) == 0) {  // Kiểm tra nút 2
//             printf("Button 2 pressed!\n");
//         }
//         else{
//             printf("Button 2 not pressed!\n");
//         }
//         if (gpio_get_level(BUTTON_3_GPIO) == 0) {  // Kiểm tra nút 3
//             printf("Button 3 pressed!\n");
//         }
//         else{
//             printf("Button 3 not pressed!\n");
//         }
//         vTaskDelay(pdMS_TO_TICKS(100));  // Đợi 100ms để tránh hiện tượng dội nút (debouncing)
//     }
// }

// void app_main() {
//     init_buttons();  // Khởi tạo GPIO cho các nút
//     xTaskCreate(check_buttons, "check_buttons", 2048, NULL, 1, NULL);  // Tạo task kiểm tra nút
// }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <stdio.h>
// #include "driver/i2c.h"
// #include "esp_log.h"

// #define I2C_MASTER_NUM I2C_NUM_0               // I2C port number for master
// #define I2C_MASTER_SCL_IO 22                   // GPIO number for I2C SCL
// #define I2C_MASTER_SDA_IO 21                   // GPIO number for I2C SDA
// #define I2C_MASTER_FREQ_HZ 100000              // I2C master clock frequency
// #define LCD_ADDR 0x27                          // I2C address of the LCD with PCF8574
// #define WRITE_BIT I2C_MASTER_WRITE             // I2C master write
// #define ACK_CHECK_EN 0x1                       // Enable ACK check
// #define TAG "LCD1602"

// static esp_err_t i2c_master_init() {
//     i2c_config_t conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = I2C_MASTER_SDA_IO,
//         .scl_io_num = I2C_MASTER_SCL_IO,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = I2C_MASTER_FREQ_HZ,
//     };
//     ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
//     ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
//     return ESP_OK;
// }

// static esp_err_t i2c_send_data(uint8_t addr, uint8_t *data, size_t len) {
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     ESP_ERROR_CHECK(i2c_master_start(cmd));
//     ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | WRITE_BIT, ACK_CHECK_EN));
//     ESP_ERROR_CHECK(i2c_master_write(cmd, data, len, ACK_CHECK_EN));
//     ESP_ERROR_CHECK(i2c_master_stop(cmd));
//     esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//     i2c_cmd_link_delete(cmd);
//     return ret;
// }

// // Send a command to the LCD
// static esp_err_t lcd_send_command(uint8_t cmd) {
//     uint8_t upper_nibble = (cmd & 0xF0) | 0x0C; // Enable = 1, RS = 0
//     uint8_t lower_nibble = ((cmd << 4) & 0xF0) | 0x0C;

//     uint8_t data[] = {
//         upper_nibble, (upper_nibble & ~0x04), // Enable = 0
//         lower_nibble, (lower_nibble & ~0x04)  // Enable = 0
//     };

//     return i2c_send_data(LCD_ADDR, data, sizeof(data));
// }

// // Send data to the LCD
// static esp_err_t lcd_send_data(uint8_t data) {
//     uint8_t upper_nibble = (data & 0xF0) | 0x0D; // Enable = 1, RS = 1
//     uint8_t lower_nibble = ((data << 4) & 0xF0) | 0x0D;

//     uint8_t payload[] = {
//         upper_nibble, (upper_nibble & ~0x04), // Enable = 0
//         lower_nibble, (lower_nibble & ~0x04)  // Enable = 0
//     };

//     return i2c_send_data(LCD_ADDR, payload, sizeof(payload));
// }

// // Initialize the LCD
// static esp_err_t lcd_init() {
//     ESP_ERROR_CHECK(lcd_send_command(0x33)); // Initialize in 4-bit mode
//     ESP_ERROR_CHECK(lcd_send_command(0x32)); // Set to 4-bit mode
//     ESP_ERROR_CHECK(lcd_send_command(0x28)); // 2-line, 5x7 matrix
//     ESP_ERROR_CHECK(lcd_send_command(0x0C)); // Display on, cursor off
//     ESP_ERROR_CHECK(lcd_send_command(0x06)); // Increment cursor
//     ESP_ERROR_CHECK(lcd_send_command(0x01)); // Clear display
//     vTaskDelay(5 / portTICK_PERIOD_MS);
//     return ESP_OK;
// }

// // Print a string on the LCD
// static void lcd_print(const char *str) {
//     while (*str) {
//         lcd_send_data((uint8_t)*str++);
//     }
// }

// void app_main() {
//     ESP_LOGI(TAG, "Initializing I2C...");
//     ESP_ERROR_CHECK(i2c_master_init());

//     ESP_LOGI(TAG, "Initializing LCD...");
//     ESP_ERROR_CHECK(lcd_init());
    
//     ESP_LOGI(TAG, "Displaying message...");
//     lcd_print(" NOOOBBBBB");
// }






#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define IN1_GPIO 18  // Chân điều khiển IN1 của L298N
#define IN2_GPIO 19  // Chân điều khiển IN2 của L298N
#define PWM_GPIO 21  // Chân điều khiển PWM (EN A hoặc EN B)

#define TAG "MotorControl"

// Hàm khởi tạo các chân GPIO và MCPWM
void motor_init(void) {
    // Cấu hình GPIO cho IN1 và IN2
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IN1_GPIO) | (1ULL << IN2_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Cấu hình MCPWM cho PWM
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PWM_GPIO);
    mcpwm_config_t pwm_config = {
        .frequency = 1000,               // Tần số PWM: 1kHz
        .cmpr_a = 0,                     // Chu kỳ làm việc ban đầu: 0%
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,  // MCPWM duty mode
        .counter_mode = MCPWM_UP_COUNTER
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    ESP_LOGI(TAG, "Motor initialized");
}

// Hàm điều khiển động cơ
void motor_control(float speed, bool direction) {
    // Kiểm tra giá trị speed nằm trong khoảng 0-100
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 255;

    // Đặt hướng động cơ
    if (direction) {
        gpio_set_level(IN1_GPIO, 1);
        gpio_set_level(IN2_GPIO, 0);
    } else {
        gpio_set_level(IN1_GPIO, 0);
        gpio_set_level(IN2_GPIO, 1);
    }

    // Điều chỉnh tốc độ động cơ bằng PWM
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, speed);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);

    ESP_LOGI(TAG, "Motor speed: %.2f%%, direction: %s", speed, direction ? "Forward" : "Reverse");
}

void test_motor_no_pwm() {
    gpio_set_level(IN1_GPIO, 1); // Tiến
    gpio_set_level(IN2_GPIO, 0);
    gpio_set_level(PWM_GPIO, 1); // ENA cấp mức cao
    //vTaskDelay(pdMS_TO_TICKS(5000));
}


// Hàm ví dụ sử dụng
void app_main(void) {
    motor_init();

    while (1) {
        // Chạy động cơ tiến với tốc độ 50%
        motor_control(101, true);
        //vTaskDelay(pdMS_TO_TICKS(3000))

    }
}
