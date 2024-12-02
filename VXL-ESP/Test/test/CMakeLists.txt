idf_component_register(SRCS
                    INCLUDE_DIRS ""
                    REQUIRES esp_wifi nvs_flash esp_http_client wifi_connect http_client)

if(DEFINED ENV{WIFI_SSID})
    message(STATUS "Environment variable WIFI_SSID is defined: $ENV{WIFI_SSID}")
    add_compile_definitions(-DWIFI_SSID=$ENV{WIFI_SSID})
else()
    message(WARNING "Environment variable WIFI_SSID is not defined")
endif()

if(DEFINED ENV{WIFI_PASSWORD})
    message(STATUS "Environment variable WIFI_PASSWORD is defined")
    add_compile_definitions(-DWIFI_PASSWORD=$ENV{WIFI_PASSWORD})
else()
    message(WARNING "Environment variable WIFI_PASSWORD is not defined")
endif()

if(DEFINED ENV{HOST})
    message(STATUS "Environment variable HOST is defined to $ENV{HOST}")
    add_compile_definitions(-DHOST=$ENV{HOST})
endif()

if(DEFINED ENV{PORT})
    message(STATUS "Environment variable PORT is defined to $ENV{PORT}")
    add_compile_definitions(-DPORT=$ENV{PORT})
endif()