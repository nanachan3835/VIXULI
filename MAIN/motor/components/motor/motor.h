#ifndef MOTOR_H
#define MOTOR_H


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

//chân motor
#define MOTOR_IN1_GPIO  GPIO_NUM_25//25  // GPIO điều khiển hướng 1
#define MOTOR_IN2_GPIO  GPIO_NUM_26//26  // GPIO điều khiển hướng 2
#define MOTOR_EN_GPIO   GPIO_NUM_27//27  // GPIO điều chỉnh tốc độ (PWM)

// Cấu hình LEDC (PWM)
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_OUTPUT_IO      MOTOR_EN_GPIO
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_FREQUENCY      5000 // Tần số PWM: 5 kHz
#define LEDC_RESOLUTION     LEDC_TIMER_8_BIT // Độ phân giải PWM: 8-bit


extern volatile int motor_speed ; // Tốc độ (-255 đến 255)

// Cờ trạng thái quay chiều
extern volatile int last_clk_state ;



void motor_control_init();
void motor_set_speed(int speed);





#endif