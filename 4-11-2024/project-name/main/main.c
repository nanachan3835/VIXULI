#include <stdio.h>
#include<FreeRTOS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"


// GPIO của các nút bấm, động cơ, và LCD I2C
#define BUTTON1_PIN GPIO_NUM_21
#define BUTTON2_PIN GPIO_NUM_22
#define BUTTON3_PIN GPIO_NUM_23
#define ROTARY_A_PIN GPIO_NUM_25
#define ROTARY_B_PIN GPIO_NUM_26
#define MOTOR_PIN_IN1 GPIO_NUM_18
#define MOTOR_PIN_IN2 GPIO_NUM_19
#define MOTOR_PIN_ENA GPIO_NUM_5


bool motor_on = false;
bool direction_forward = true;
bool speed_control_enabled = false;
int motor_speed = 50; 


void motor_pwm_init(void) {
    ledc_timer_config_t pwm_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = PWM_FREQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = PWM_TIMER,
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .channel = PWM_CHANNEL,
        .duty = 0,
        .gpio_num = MOTOR_PIN_ENA,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = PWM_TIMER,
    };
    ledc_channel_config(&pwm_channel);
}

// Điều chỉnh tốc độ động cơ
void set_motor_speed(int speed) {
    uint32_t duty = (speed * 255) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);
}

// Điều chỉnh chiều quay
void set_motor_direction(bool forward) {
    gpio_set_level(MOTOR_PIN_IN1, forward ? 1 : 0);
    gpio_set_level(MOTOR_PIN_IN2, forward ? 0 : 1);
}

// Kiểm tra nút bấm
bool read_button(gpio_num_t pin) {
    return gpio_get_level(pin) == 0;
}

// Khởi tạo GPIO
void motor_gpio_init(void) {
    gpio_set_direction(MOTOR_PIN_IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_PIN_IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON1_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON2_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON3_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON1_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(BUTTON2_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(BUTTON3_PIN, GPIO_PULLUP_ONLY);
}

// Xử lý các nút bấm
void handle_buttons() {
    if (read_button(BUTTON1_PIN)) {
        motor_on = !motor_on;
        if (motor_on) {
            printf("Motor ON\n");
            set_motor_speed(motor_speed);
        } else {
            printf("Motor OFF\n");
            set_motor_speed(0);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS); // Khử rung
    }

    if (read_button(BUTTON2_PIN)) {
        direction_forward = !direction_forward;
        printf("Direction: %s\n", direction_forward ? "Forward" : "Reverse");
        set_motor_direction(direction_forward);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    if (read_button(BUTTON3_PIN)) {
        speed_control_enabled = !speed_control_enabled;
        printf("Speed Control: %s\n", speed_control_enabled ? "Enabled" : "Disabled");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

// Xử lý tốc độ từ rotary encoder
void handle_rotary_encoder() {
    static int last_a = 1;
    int a = gpio_get_level(ROTARY_A_PIN);
    int b = gpio_get_level(ROTARY_B_PIN);

    if (speed_control_enabled && a == 0 && last_a == 1) {
        if (b == 0 && motor_speed < 100) {
            motor_speed += 5; // Tăng tốc
        } else if (b == 1 && motor_speed > 0) {
            motor_speed -= 5; // Giảm tốc
        }
        printf("Speed: %d%%\n", motor_speed);
        set_motor_speed(motor_on ? motor_speed : 0);
    }
    last_a = a;
}










void app_main(void) {
    motor_gpio_init();
    motor_pwm_init();
    set_motor_direction(direction_forward);

    while (1) {
        handle_buttons();
        handle_rotary_encoder();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
