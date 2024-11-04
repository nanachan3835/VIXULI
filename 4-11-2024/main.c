#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "motor.c"
#include "lcd.c"






void app_main(void) {
    motor_gpio_init();
    motor_pwm_init();
    lcd_init();
    set_motor_direction(direction_forward);
    lcd_display_info();

    while (1) {
        handle_buttons();
        handle_rotary_encoder();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
