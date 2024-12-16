#include <stdio.h>
#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "state.h"
#include "driver/adc.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_adc_cal.h"
//#include "driver/pcnt.h"



// Cấu hình GPIO
// GPIO điều khiển motor
#define MOTOR_IN1_GPIO  GPIO_NUM_25//25  // GPIO điều khiển hướng 1
#define MOTOR_IN2_GPIO  GPIO_NUM_26//26  // GPIO điều khiển hướng 2
#define MOTOR_EN_GPIO   GPIO_NUM_27//27  // GPIO điều chỉnh tốc độ (PWM)


//nut bam
#define BUTTON_PIN_1 GPIO_NUM_12 // Nút bấm
//#define LED_PIN GPIO_NUM_2    // LED
#define BUTTON_PIN_2 GPIO_NUM_4 // Nút bấm
#define BUTTON_PIN_3 GPIO_NUM_5 // Nút bấm

// Cấu hình LEDC (PWM)
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_OUTPUT_IO      MOTOR_EN_GPIO
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_FREQUENCY      5000 // Tần số PWM: 5 kHz
#define LEDC_RESOLUTION     LEDC_TIMER_8_BIT // Độ phân giải PWM: 8-bit

//cau hinh num xoay
//#define ROTARY_CLK_GPIO    GPIO_NUM_18     //18 // GPIO cho tín hiệu CLK từ rotary encoder
//#define ROTARY_DT_GPIO     GPIO_NUM_19   //19 // GPIO cho tín hiệu DT từ rotary encoder    
#define EXAMPLE_PCNT_HIGH_LIMIT 255//100
#define EXAMPLE_PCNT_LOW_LIMIT  -255//-100
//#define POT_PIN GPIO_NUM_34 
#define EXAMPLE_EC11_GPIO_A 0
#define EXAMPLE_EC11_GPIO_B 2
volatile int speed_control_enabled = 1; // Cờ cho phép điều khiển tốc độ

/////hàm khởi tạo các chân linh kiện cho esp32
/////////////////hàm khoi tao điều khiển motor


// Biến lưu trạng thái tốc độ động cơ
volatile int motor_speed = 0; // Tốc độ (-255 đến 255)

// Cờ trạng thái quay chiều
volatile int last_clk_state = 0;
//
static const char *TAG = "MOTOR";

///////////////////////////////////////////////////////////////////

#define ADC_CHANNEL    ADC1_CHANNEL_0 // GPIO36 (VP) by default
#define ADC_WIDTH      ADC_WIDTH_BIT_12
#define ADC_ATTEN      ADC_ATTEN_DB_11
#define DEFAULT_VREF   1100 // Default VREF in mV (set according to your ESP32 board)

static esp_adc_cal_characteristics_t *adc_chars;


/////////////////////////////////////////////////////////////////



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






void configure_button() {
    gpio_set_direction(BUTTON_PIN_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_1, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTTON_PIN_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_2, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTTON_PIN_3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN_3, GPIO_PULLUP_ONLY);
}


////////////////////////////////////////////////////////////////////
//hàm sử lý logic

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

////////////////////////////////////////////////
// Hàm xử lý tín hiệu từ rotary encoder (ngắt)
// static void IRAM_ATTR rotary_isr_handler(void* arg) {
//     if (!speed_control_enabled) 
//     {
//         return;
//     }


//     int clk_state = gpio_get_level(ROTARY_CLK_GPIO);
//     int dt_state = gpio_get_level(ROTARY_DT_GPIO);

//     if (clk_state != last_clk_state) {
//         if (dt_state != clk_state) {
//             motor_speed += 10; // Tăng tốc
//         } else {
//             motor_speed -= 10; // Giảm tốc
//         }

//         // Giới hạn tốc độ trong khoảng [-255, 255]
//         if (motor_speed > 255) motor_speed = 255;
//         if (motor_speed < -255) motor_speed = -255;

//         last_clk_state = clk_state;
//     }
// }



// // Hàm khởi tạo Rotary Encoder
// void rotary_encoder_init() {
//     // Cấu hình GPIO cho CLK và DT
//     gpio_config_t io_conf = {
//         .pin_bit_mask = (1ULL << ROTARY_CLK_GPIO) | (1ULL << ROTARY_DT_GPIO),
//         .mode = GPIO_MODE_INPUT,
//         .pull_up_en = GPIO_PULLUP_ENABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_ANYEDGE
//     };
//     gpio_config(&io_conf);

//     // Đăng ký ngắt cho CLK
//     gpio_install_isr_service(0);
//     gpio_isr_handler_add(ROTARY_CLK_GPIO, rotary_isr_handler, NULL);

//     // Lấy trạng thái ban đầu của CLK
//     last_clk_state = gpio_get_level(ROTARY_CLK_GPIO);
// }







/// dung num xoay de dieu chinh toc do motor
// static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
// {
//     BaseType_t high_task_wakeup;
//     QueueHandle_t queue = (QueueHandle_t)user_ctx;
//     // send event data to queue, from this interrupt callback
//     xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
//     return (high_task_wakeup == pdTRUE);
// }



// Function to convert voltage to speed
int convert_voltage_to_speed(uint32_t voltage) {
    int speed;
    if (voltage <= 1650) {
        // Map 0 to 1650 mV to -255 to 0
        speed = (voltage * -255) / 1650;
    } else {
        // Map 1650 to 3300 mV to 0 to 255
        speed = ((voltage - 1650) * 255) / 1650;
    }
    return speed;
}
//////////////////////////////////////////////////////////////////////////






/////////////////////////ham xử lú sự kiện/////////
//extern "C" void app_main(void)
void event_handler(void)
{
//     //khai bao num xoay
//    ESP_LOGI(TAG, "install pcnt unit");
//     pcnt_unit_config_t unit_config = {
//         .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
//         .high_limit = EXAMPLE_PCNT_HIGH_LIMIT
//         //.low_limit = EXAMPLE_PCNT_LOW_LIMIT
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
//     #endif

//     // Report counter value
//     int pulse_count = 0;
//     int event_count = 0;

    //configure_adc();
    //rotary_encoder_init();


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //sudung potentiometer de dieu chinh toc do motor
     // Allocate memory for ADC characteristics
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

    // Configure ADC width and attenuation
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars);

    printf("ADC configured. Reading values...\n");
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    motor_control_init();
    //StateofMotor Motor_state_1(0,0,0,0,0);
    configure_button();
    /////////////////////////////////////////////////main loop////////////////////
    while(1)
    {




    int raw_value = adc1_get_raw(ADC_CHANNEL);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);
    printf("Raw: %d\tVoltage: %" PRIu32 "mV\tSpeed: %d\n", raw_value, voltage, motor_speed);
    


        ///khoi tao logic cho nut bam
    //Motor_state_1.setButton1(button1_logic());
    //Motor_state_1.setButton2(button2_logic());
    //Motor_state_1.setButton3(button3_logic());
    if(gpio_get_level(BUTTON_PIN_1) == 1)
        //Motor_state_1.CheckButton1() == 0)
    {
        printf("Button 1 is pressed\n");
        speed_control_enabled = 0;
        if (gpio_get_level(BUTTON_PIN_2) == 0)
            //Motor_state_1.CheckButton2() == 1)
        {
            printf("Button 2 is not pressed\n");

            motor_set_speed(motor_speed);
            vTaskDelay(100 / portTICK_PERIOD_MS);   
        }
        else
        {
            printf("Button 2 is not pressed\n");
            motor_set_speed(0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        printf("Button 1 is pressed\n");
        speed_control_enabled = 0;
        if (gpio_get_level(BUTTON_PIN_2) == 0)
            //Motor_state_1.CheckButton2() == 1)
        {
            printf("Button 2 is not pressed\n");
            speed_control_enabled = 1;
            while(1)
            {
        //         if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(1000))) {
        //                 ESP_LOGI(TAG, "Watch point event, count: %d", event_count);
        //             } 
        // else {
        //     ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
        //     ESP_LOGI(TAG, "Pulse count: %d", pulse_count);
        // }
        //         //rotary_encoder_init();
        //         motor_speed = pulse_count;
                motor_speed = convert_voltage_to_speed(voltage);
                motor_set_speed(motor_speed);
                printf("Raw: %d\tVoltage: %" PRIu32 "mV\tSpeed: %d\n", raw_value, voltage, motor_speed);
                if(gpio_get_level(BUTTON_PIN_2) == 1)
                    //Motor_state_1.CheckButton2() == 0)
                {
                    printf("Button 2 is pressed\n");
                    break;
                }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else
        {
            printf("Button 2 is not pressed\n");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

    }
}
}



////////////////////////////////////////////////////////////////////////////
//
extern "C" void app_main(void)
{
    event_handler();
}