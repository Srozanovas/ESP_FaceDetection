idf_component_register(SRCS "Source/main.c" 
                    "Source/wifi.c"
                    "Source/http_server.c" 
                    "Source/camera.c" 
                   
                    PRIV_REQUIRES spi_flash
                    REQUIRES esp_wifi
                    REQUIRES nvs_flash
                    REQUIRES esp-tls
                    REQUIRES esp_http_server
                    REQUIRES esp_eth
                    REQUIRES esp_driver_ledc
                    REQUIRES esp_driver_gpio
                    
                  
                    INCLUDE_DIRS "")
