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

systemConfig mainConfig;
eFiniteState FSM = eFSMImageGet;


void app_main(void) {
    mainConfig.faceDetectEnable = 0;
    mainConfig.streamEnable = 0;
    mainConfig.network = eTensorFlow;
    mainConfig.res[0] = FRAMESIZE_240X240;
    mainConfig.res[1] = FRAMESIZE_240X240;

    CameraInit();
    WifiInit();
    HTTPServerStart();
}
