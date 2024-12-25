#pragma once

#ifndef LCD2024_H
#define LCD2024_H


#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_NUM I2C_NUM_0               // I2C port number for master
#define I2C_MASTER_SCL_IO 22                   // GPIO number for I2C SCL
#define I2C_MASTER_SDA_IO 21                   // GPIO number for I2C SDA
#define I2C_MASTER_FREQ_HZ 100000              // I2C master clock frequency
#define LCD_ADDR 0x27                          // I2C address of the LCD with PCF8574
#define WRITE_BIT I2C_MASTER_WRITE             // I2C master write
#define ACK_CHECK_EN 0x1                       // Enable ACK check
#define TAG "LCD1602"



static esp_err_t i2c_master_init();
static esp_err_t i2c_send_data(uint8_t addr, uint8_t *data, size_t len);
static esp_err_t lcd_send_command(uint8_t cmd);
static esp_err_t lcd_send_data(uint8_t data);
static esp_err_t lcd_init();   
static void lcd_print(const char *str);

#endif