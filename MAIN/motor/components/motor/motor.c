#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "motor.h"
// //chân motor
// #define MOTOR_IN1_GPIO  GPIO_NUM_25//25  // GPIO điều khiển hướng 1
// #define MOTOR_IN2_GPIO  GPIO_NUM_26//26  // GPIO điều khiển hướng 2
// #define MOTOR_EN_GPIO   GPIO_NUM_27//27  // GPIO điều chỉnh tốc độ (PWM)

// // Cấu hình LEDC (PWM)
// #define LEDC_TIMER          LEDC_TIMER_0
// #define LEDC_MODE           LEDC_HIGH_SPEED_MODE
// #define LEDC_OUTPUT_IO      MOTOR_EN_GPIO
// #define LEDC_CHANNEL        LEDC_CHANNEL_0
// #define LEDC_FREQUENCY      5000 // Tần số PWM: 5 kHz
// #define LEDC_RESOLUTION     LEDC_TIMER_8_BIT // Độ phân giải PWM: 8-bit


volatile int motor_speed = 0; // Tốc độ (-255 đến 255)

// Cờ trạng thái quay chiều
volatile int last_clk_state = 0;



void motor_control_init() {
    // Cấu hình Timer cho LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_RESOLUTION,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Cấu hình kênh PWM cho LEDC
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = LEDC_OUTPUT_IO,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // Bắt đầu với duty cycle 0
        .hpoint         = 0
    };

 ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
    // Cấu hình GPIO điều khiển hướng
    gpio_set_direction(MOTOR_IN1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_IN2_GPIO, GPIO_MODE_OUTPUT);
}

//hàm đặt tốc độ cho motor

void motor_set_speed(int speed) {
    // speed: giá trị từ -255 đến 255
    if (speed > 0) {
        gpio_set_level(MOTOR_IN1_GPIO, 1);
        gpio_set_level(MOTOR_IN2_GPIO, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, speed);
    } else if (speed < 0) {
        gpio_set_level(MOTOR_IN1_GPIO, 0);
        gpio_set_level(MOTOR_IN2_GPIO, 1);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, -speed);
    } else {
        gpio_set_level(MOTOR_IN1_GPIO, 0);
        gpio_set_level(MOTOR_IN2_GPIO, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    }
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

