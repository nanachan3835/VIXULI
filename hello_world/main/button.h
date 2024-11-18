#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// Define button GPIO pins
#define BUTTON_1_GPIO 18  // Adjust the GPIO pin number as needed
#define BUTTON_2_GPIO 19  // Adjust the GPIO pin number as needed
#define BUTTON_3_GPIO 21  // Adjust the GPIO pin number as needed
#define MOTOR_CONTROL_GPIO 22  // Adjust the GPIO pin for motor control as needed
 
// Function to initialize the button GPIOs
void init_buttons() {
    gpio_config_t io_conf;

    // Configure button 1
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_1_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Configure button 2
    io_conf.pin_bit_mask = (1ULL << BUTTON_2_GPIO);
    gpio_config(&io_conf);

    // Configure button 3
    io_conf.pin_bit_mask = (1ULL << BUTTON_3_GPIO);
    gpio_config(&io_conf);
}


// Function to read data from the buttons
int receive_data_button(int button_gpio) {
    // Read the state of the specified button
    int button_state = gpio_get_level(button_gpio);
    return button_state;
}




// Variable to track the motor state
static bool motor_on = false;

// Function to handle the logic of Button 1
void button1_logic() {
    // Read the state of Button 1
    int button1_state = receive_data_button(BUTTON_1_GPIO);

    // Check if Button 1 is pressed
    if (button1_state == 1) {
        // Toggle motor state
        motor_on = !motor_on;

        // Update motor control
        if (motor_on) {
            // Turn the motor ON
            gpio_set_level(MOTOR_CONTROL_GPIO, 1);
            printf("Motor is now ON\n");
        } else {
            // Turn the motor OFF
            gpio_set_level(MOTOR_CONTROL_GPIO, 0);
            printf("Motor is now OFF\n");
        }

        // Debounce delay to prevent multiple triggers
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}



// Variables to track button states and motor status
static bool motor_paused = false;
static bool motor_on = false;

// Function to handle the logic of Button 2
void button2_logic() {
    // Read the state of Button 2
    int button2_state = receive_data_button(BUTTON_2_GPIO);

    // Check if Button 2 is pressed
    if (button2_state == 1) {
        if (motor_on) {
            // Toggle motor pause/resume
            motor_paused = !motor_paused;

            if (motor_paused) {
                // Pause the motor and save data to Firebase
                gpio_set_level(MOTOR_CONTROL_GPIO, 0);
                printf("Motor is paused, data saved to Firebase\n");
                // Add Firebase saving logic here
            } else {
                // Resume the motor
                gpio_set_level(MOTOR_CONTROL_GPIO, 1);
                printf("Motor has resumed\n");
            }
        } else {
            // If Button 1 is OFF, handle setting motor speed
            printf("Button 2 pressed: Adjusting motor speed\n");
            // Add motor speed setting logic if needed
        }

        // Debounce delay to prevent multiple triggers
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}



static bool motor_direction_forward = true;  // Default direction

// Function to handle the logic of Button 3
void button3_logic() {
    // Read the state of Button 3
    int button3_state = receive_data_button(BUTTON_3_GPIO);

    // Check if Button 3 is pressed
    if (button3_state == 1) {
        // Toggle motor direction
        motor_direction_forward = !motor_direction_forward;

        // Update motor direction
        if (motor_direction_forward) {
            printf("Motor direction set to FORWARD\n");
        } else {
            printf("Motor direction set to REVERSE\n");
        }

        // Implement any hardware-specific logic for direction change here

        // Debounce delay to prevent multiple triggers
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


// Function to control motor based on button signals using L298
void motor_control() {
    // Check button 1 logic (turn motor on/off)
    button1_logic();

    // Check button 2 logic (pause/resume motor)
    button2_logic();

    // Check button 3 logic (change motor direction)
    button3_logic();

    // Implement motor control logic based on button states
    if (motor_on && !motor_paused) {
        if (motor_direction_forward) {
            // Set motor to run forward
            gpio_set_level(MOTOR_IN1_GPIO, 1);
            gpio_set_level(MOTOR_IN2_GPIO, 0);
            printf("Motor is running FORWARD\n");
        } else {
            // Set motor to run in reverse
            gpio_set_level(MOTOR_IN1_GPIO, 0);
            gpio_set_level(MOTOR_IN2_GPIO, 1);
            printf("Motor is running in REVERSE\n");
        }
    } else {
        // Motor is OFF or paused
        gpio_set_level(MOTOR_IN1_GPIO, 0);
        gpio_set_level(MOTOR_IN2_GPIO, 0);
        if (!motor_on) {
            printf("Motor is OFF\n");
        } else if (motor_paused) {
            printf("Motor is PAUSED\n");
        }
    }
}

void init_motor_control() {
    gpio_pad_select_gpio(MOTOR_IN1_GPIO);
    gpio_set_direction(MOTOR_IN1_GPIO, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(MOTOR_IN2_GPIO);
    gpio_set_direction(MOTOR_IN2_GPIO, GPIO_MODE_OUTPUT);
}



