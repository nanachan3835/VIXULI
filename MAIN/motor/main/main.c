
#include <stdio.h>
#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/adc.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_adc_cal.h"
//#include "driver/pcnt.h"
//#include "lcd.h"
#include "driver/i2c.h"
#include "string.h"
#include "wifi_connect.h"





//lcd 1602
#define I2C_MASTER_NUM I2C_NUM_0               // I2C port number for master
#define I2C_MASTER_SCL_IO 22                   // GPIO number for I2C SCL
#define I2C_MASTER_SDA_IO 21                   // GPIO number for I2C SDA
#define I2C_MASTER_FREQ_HZ 100000              // I2C master clock frequency
#define LCD_ADDR 0x27                          // I2C address of the LCD with PCF8574
#define WRITE_BIT I2C_MASTER_WRITE             // I2C master write
#define ACK_CHECK_EN 0x1                       // Enable ACK check
//#define TAG "LCD1602"


// Cấu hình GPIO
// GPIO điều khiển motor
#define MOTOR_IN1_GPIO  GPIO_NUM_25//25  // GPIO điều khiển hướng 1
#define MOTOR_IN2_GPIO  GPIO_NUM_26//26  // GPIO điều khiển hướng 2
#define MOTOR_EN_GPIO   GPIO_NUM_27//27  // GPIO điều chỉnh tốc độ (PWM)


//nut bam
#define BUTTON_PIN_1 GPIO_NUM_15// Nút bấm 1
//#define LED_PIN GPIO_NUM_2    // LED
#define BUTTON_PIN_2 GPIO_NUM_4 // Nút bấm 2
#define BUTTON_PIN_3 GPIO_NUM_5 // Nút bấm 3

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
volatile float motor_speed = 0; // Tốc độ (-255 đến 255)

// Cờ trạng thái quay chiều
volatile int last_clk_state = 0;
//
static const char *TAG = "MOTOR";

///////////////////////////////////////////////////////////////////
//potentiometer
#define ADC_CHANNEL    ADC1_CHANNEL_0 // GPIO36 (VP) by default
#define ADC_WIDTH      ADC_WIDTH_BIT_12
#define ADC_ATTEN      ADC_ATTEN_DB_12
#define DEFAULT_VREF   1100 // Default VREF in mV (set according to your ESP32 board)

static esp_adc_cal_characteristics_t *adc_chars;


/////////////////////////////////////////////////////////////////
//lcd 1602


static esp_err_t i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
    return ESP_OK;
}

static esp_err_t i2c_send_data(uint8_t addr, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write(cmd, data, len, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Send a command to the LCD
static esp_err_t lcd_send_command(uint8_t cmd) {
    uint8_t upper_nibble = (cmd & 0xF0) | 0x0C; // Enable = 1, RS = 0
    uint8_t lower_nibble = ((cmd << 4) & 0xF0) | 0x0C;

    uint8_t data[] = {
        upper_nibble, (upper_nibble & ~0x04), // Enable = 0
        lower_nibble, (lower_nibble & ~0x04)  // Enable = 0
    };

    return i2c_send_data(LCD_ADDR, data, sizeof(data));
}

// Send data to the LCD
static esp_err_t lcd_send_data(uint8_t data) {
    uint8_t upper_nibble = (data & 0xF0) | 0x0D; // Enable = 1, RS = 1
    uint8_t lower_nibble = ((data << 4) & 0xF0) | 0x0D;

    uint8_t payload[] = {
        upper_nibble, (upper_nibble & ~0x04), // Enable = 0
        lower_nibble, (lower_nibble & ~0x04)  // Enable = 0
    };

    return i2c_send_data(LCD_ADDR, payload, sizeof(payload));
}

// Initialize the LCD
static esp_err_t lcd_init() {
    ESP_ERROR_CHECK(lcd_send_command(0x33)); // Initialize in 4-bit mode
    ESP_ERROR_CHECK(lcd_send_command(0x32)); // Set to 4-bit mode
    ESP_ERROR_CHECK(lcd_send_command(0x28)); // 2-line, 5x7 matrix
    ESP_ERROR_CHECK(lcd_send_command(0x0C)); // Display on, cursor off
    ESP_ERROR_CHECK(lcd_send_command(0x06)); // Increment cursor
    ESP_ERROR_CHECK(lcd_send_command(0x01)); // Clear display
    vTaskDelay(5 / portTICK_PERIOD_MS);
    return ESP_OK;
}




//set cursor
static esp_err_t lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address;

    // Tính địa chỉ DDRAM dựa trên dòng và cột
    switch (row) {
        case 0:
            address = 0x00 + col; // Dòng 1
            break;
        case 1:
            address = 0x40 + col; // Dòng 2
            break;
        default:
            return ESP_ERR_INVALID_ARG; // Dòng không hợp lệ
    }

    // Gửi lệnh đặt địa chỉ DDRAM
    return lcd_send_command(0x80 | address);
}

//clear lcd
static esp_err_t lcd_clear() {
    esp_err_t ret = lcd_send_command(0x01); // Clear display command
    vTaskDelay(5 / portTICK_PERIOD_MS);     // Wait for the command to complete
    return ret;
}

// Print a string on the LCD
static esp_err_t lcd_print(const char *str) {
    lcd_clear();
    while (*str) {
        ESP_ERROR_CHECK(lcd_send_data((uint8_t)*str++));
    }
    return ESP_OK;
}








///////////////////////////////////////////////


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

//////////////////test nut bam////////////////////////
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












///////////////////////////////////////////////////// Function to convert voltage to speed
float convert_voltage_to_speed(uint32_t voltage){
    float speed;
    speed = ((voltage-3129)* -268)/3129;
    return speed;
}
//////////////////////////////////////////////////////////////////////////

// void motor_set_speed_by_button(int speed)
// {
//     if(speed_control_enabled == 1)
//     {
//         motor_set_speed(speed);
//     }
//     else
//     {
//         motor_set_speed(0);
//     }
//}




/////////////////////////ham xử lú sự kiện/////////
//extern "C" void app_main(void)
void event_handler(void)
{
/////////////lcd 
    ESP_LOGI(TAG, "Initializing I2C...");
    ESP_ERROR_CHECK(i2c_master_init());

    ESP_LOGI(TAG, "Initializing LCD...");
    ESP_ERROR_CHECK(lcd_init());
    
    ESP_LOGI(TAG, "Displaying message...");
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
    wifi_init_dhcp();
    motor_control_init();
    configure_button();
    /////////////////////////////////////////////////main loop////////////////////
    while(1)
    {
        printf("Button 1: %d\n", button1_logic());
        printf("Button 2: %d\n", button2_logic());
        printf("Button 3: %d\n", button3_logic());

///////////////////////////////in lcd
    char buffer[15];
    snprintf(buffer,sizeof(buffer) ,"%f", motor_speed);
    char str[] = "  Speed:";
    char results_string[25];
    snprintf(results_string, sizeof(results_string), "%s%s", str,buffer);
    lcd_print(results_string);
    ////////////////
    int raw_value = adc1_get_raw(ADC_CHANNEL);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);
    //motor_speed = convert_voltage_to_speed(voltage);
    // printf("Raw: %d\tVoltage: %" PRIu32 "mV\tSpeed: %f\n", raw_value, voltage, motor_speed);
    ////////////////////////////////////////////////////////
    
    if(gpio_get_level(BUTTON_PIN_1) == 1)
    {
        //printf("Button 1 is pressed\n");
        //speed_control_enabled = 0;
        if (gpio_get_level(BUTTON_PIN_2) == 0)
        {
            //printf("Button 2 is pressed\n");

            //motor_set_speed(motor_speed);

            lcd_print("  data sended");
            vTaskDelay(1000 / portTICK_PERIOD_MS);   
        }
        else
        {
            //printf("Button 2 is not pressed\n");
            //motor_set_speed(0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        //printf("Button 1 is not pressed\n");
        //printf("speed_adjust_enabled: %d\n", speed_control_enabled);
        //speed_control_enabled = 0;
        if (gpio_get_level(BUTTON_PIN_2) == 0)
        {
            //printf("Button 2 is pressed\n");
            //speed_control_enabled = 1;
            while(1)
            {
                // motor_speed = convert_voltage_to_speed(voltage);
                // motor_set_speed(motor_speed);
                // printf("Raw: %d\tVoltage: %" PRIu32 "mV\tSpeed: %f\n", raw_value, voltage, motor_speed);
                if(gpio_get_level(BUTTON_PIN_2) == 1)
                    //Motor_state_1.CheckButton2() == 0)
                {
                    motor_speed = convert_voltage_to_speed(voltage);
                    printf("MOTOR SPEED: %f\n", motor_speed);
                    lcd_print("   save speed");
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    break;
                }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else
        {
            //printf("Button 2 is not pressed\n");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

    }

}
}



////////////////////////////////////////////////////////////////////////////
//
void app_main(void)
{
    event_handler();
    
}