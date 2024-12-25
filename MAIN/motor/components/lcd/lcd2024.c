#include"lcd2024.h"

#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"



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

// Print a string on the LCD
static void lcd_print(const char *str) {
    while (*str) {
        lcd_send_data((uint8_t)*str++);
    }
}

