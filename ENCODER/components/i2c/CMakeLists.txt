idf_component_register(SRCS "HD44780.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_rom log freertos soc driver nvs_flash)
