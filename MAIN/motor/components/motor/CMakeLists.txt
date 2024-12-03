idf_component_register(SRCS "motor.c"
                    INCLUDE_DIRS "."
                    REQUIRES freertos driver esp_common "esp_err.h")
