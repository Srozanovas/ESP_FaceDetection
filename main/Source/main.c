#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "esp_camera.h"



#include "../Include/main.h"

systemConfig mainConfig = { 
    eTensorFlow, 
    0
};


void app_main(void)
{
    printf("Hello world!\n");
    CameraInit();
    WifiInit();
    HTTPServerStart();
}
