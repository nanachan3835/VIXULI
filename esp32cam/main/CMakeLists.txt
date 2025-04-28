set(srcs "main.c"
	     "connect_wifi.c")
    
idf_component_register(SRCS ${srcs}
                    PRIV_REQUIRES spi_flash nvs_flash esp_driver_gpio esp32-camera-master esp_http_server esp_timer esp_wifi
                    INCLUDE_DIRS ".")
