idf_component_register(SRCS "test_compo.c"
                       INCLUDE_DIRS "."
                       REQUIRES http_server http_client wifi_connect json esp_driver_gpio lwip esp_netif)