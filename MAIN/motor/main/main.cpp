#include <stdio.h>
#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
//#include "state.h"
#include "driver/adc.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_adc_cal.h"
//#include "driver/pcnt.h"
#include "lcd2024.h"
#include "driver/i2c.h"


//lcd 1602
// #define I2C_MASTER_NUM I2C_NUM_0               // I2C port number for master
// #define I2C_MASTER_SCL_IO 22                   // GPIO number for I2C SCL
// #define I2C_MASTER_SDA_IO 21                   // GPIO number for I2C SDA
// #define I2C_MASTER_FREQ_HZ 100000              // I2C master clock frequency
// #define LCD_ADDR 0x27                          // I2C address of the LCD with PCF8574
// #define WRITE_BIT I2C_MASTER_WRITE             // I2C master write
// #define ACK_CHECK_EN 0x1                       // Enable ACK check
// #define TAG "LCD1602"


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
/////////////lcd 
    // ESP_LOGI(TAG, "Initializing I2C...");
    // ESP_ERROR_CHECK(i2c_master_init());

    // ESP_LOGI(TAG, "Initializing LCD...");
    // ESP_ERROR_CHECK(lcd_init());
    
    // ESP_LOGI(TAG, "Displaying message...");
/////////////////////////////////////////////////////////////

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


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    motor_control_init();
    //StateofMotor Motor_state_1(0,0,0,0,0);
    configure_button();
    /////////////////////////////////////////////////main loop////////////////////

    //lcd_print(" NOOOBBBBB");
    while(1)
    {
    int raw_value = adc1_get_raw(ADC_CHANNEL);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);
    printf("Raw: %d\tVoltage: %" PRIu32 "mV\tSpeed: %d\n", raw_value, voltage, motor_speed);
    


    if(gpio_get_level(BUTTON_PIN_1) == 0)
        //Motor_state_1.CheckButton1() == 0)
    {
        printf("Button 1 is pressed\n");
        speed_control_enabled = 0;
        if (gpio_get_level(BUTTON_PIN_2) == 0)
            //Motor_state_1.CheckButton2() == 1)
        {
            printf("Button 2 is pressed\n");

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
        printf("Button 1 is not pressed\n");
        speed_control_enabled = 0;
        if (gpio_get_level(BUTTON_PIN_2) == 0)
            //Motor_state_1.CheckButton2() == 1)
        {
            printf("Button 2 is pressed\n");
            speed_control_enabled = 1;
            while(1)
            {

                motor_speed = convert_voltage_to_speed(voltage);
                motor_set_speed(motor_speed);
                printf("Raw: %d\tVoltage: %" PRIu32 "mV\tSpeed: %d\n", raw_value, voltage, motor_speed);
                if(gpio_get_level(BUTTON_PIN_2) == 1)
                    //Motor_state_1.CheckButton2() == 0)
                {
                    printf("Button 2 is not pressed\n");
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