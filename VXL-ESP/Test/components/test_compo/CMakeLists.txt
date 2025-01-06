idf_component_register(SRCS "test_compo.c"
                       INCLUDE_DIRS "."
                       REQUIRES http_server http_client wifi_connect json driver freertos lwip esp_netif RotaryEncoder)

target_link_libraries(${COMPONENT_LIB} PUBLIC stdc++)