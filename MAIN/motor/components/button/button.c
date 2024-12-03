#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "button.h"


volatile int speed_control_enabled = 1; // Cờ cho phép điều khiển tốc độ

/////hàm khởi tạo các chân linh kiện cho esp32
/////////////////hàm khoi tao điều khiển motor


// Biến lưu trạng thái tốc độ động cơ
volatile int motor_speed = 0; // Tốc độ (-255 đến 255)

// Cờ trạng thái quay chiều
volatile int last_clk_state = 0;

void configure_button() {
    gpio_set_direction(BUTTON_PIN_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_1, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTTON_PIN_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_2, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTTON_PIN_3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_3, GPIO_PULLUP_ONLY);
}

int button1_logic()
{
    if (gpio_get_level(BUTTON_PIN_1) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    } 
        
}

int button2_logic()
{
    if (gpio_get_level(BUTTON_PIN_2) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
}
}
int button3_logic()
{
    if (gpio_get_level(BUTTON_PIN_3) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


// Hàm xử lý tín hiệu từ rotary encoder (ngắt)
static void IRAM_ATTR rotary_isr_handler(void* arg) {
    if (!speed_control_enabled) 
    {
        return;
    }


    int clk_state = gpio_get_level(ROTARY_CLK_GPIO);
    int dt_state = gpio_get_level(ROTARY_DT_GPIO);

    if (clk_state != last_clk_state) {
        if (dt_state != clk_state) {
            motor_speed += 10; // Tăng tốc
        } else {
            motor_speed -= 10; // Giảm tốc
        }

        // Giới hạn tốc độ trong khoảng [-255, 255]
        if (motor_speed > 255) motor_speed = 255;
        if (motor_speed < -255) motor_speed = -255;

        last_clk_state = clk_state;
    }
}




// Hàm khởi tạo Rotary Encoder
void rotary_encoder_init() {
    // Cấu hình GPIO cho CLK và DT
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << ROTARY_CLK_GPIO) | (1ULL << ROTARY_DT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);

    // Đăng ký ngắt cho CLK
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ROTARY_CLK_GPIO, rotary_isr_handler, NULL);

    // Lấy trạng thái ban đầu của CLK
    last_clk_state = gpio_get_level(ROTARY_CLK_GPIO);
}