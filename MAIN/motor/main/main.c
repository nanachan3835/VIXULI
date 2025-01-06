
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
#include "http_client.h"
#include "http_server.h"



void event_handler(void)
{

        
  


}



////////////////////////////////////////////////////////////////////////////
//
void app_main(void)
{
    event_handler();   
}