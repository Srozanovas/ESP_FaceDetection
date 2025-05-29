#include "esp_camera.h"
#include "../Include/main.h"
#include "FreeRTOSConfig.h"


camera_config_t config;


void CameraTask();
taskConfig cameraTask = {
    CameraTask,
    "Camera task",
    9000,
    NULL,
    tskIDLE_PRIORITY,
}; 

size_t _jpg_buf_len = 0;
uint8_t *_jpg_buf = NULL;
struct timeval _timestamp;
eFiniteState FSM = eFSMImageSend;


#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

#define LED_GPIO_NUM      21



void CameraInit(){ 

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;  // for streaming
    //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    ESP_ERROR_CHECK(esp_camera_init(&config));
    
    sensor_t *s = esp_camera_sensor_get();
    
    // // drop down frame size for higher initial frame rate
    if (config.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, mainConfig.res[1]);
    }

    


    xTaskCreate(cameraTask.pvTaskCode,
                cameraTask.pcName,
                cameraTask.uxStackDepth,
                cameraTask.pvParameters,
                cameraTask.uxPriority,
                NULL
                );

}





void CameraTask(){ 

    camera_fb_t *fb = NULL;
    sensor_t *s = esp_camera_sensor_get();
    vTaskDelay(5000/portTICK_PERIOD_MS);

    while(1) { 
        
        if (FSM == eFSMImageGet){ 
            

            if (mainConfig.res[1] != mainConfig.res[0]){ 
                mainConfig.res[0] = mainConfig.res[1];
                s->set_framesize(s, mainConfig.res[1]);
            }


            if (fb) {
                esp_camera_fb_return(fb);
                fb = NULL;
                _jpg_buf = NULL;
            } 


            fb = esp_camera_fb_get();
            if (!fb) {
                continue; 
            } 
    
            _timestamp.tv_sec = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
            // if (fb->format != PIXFORMAT_JPEG) {
            //     bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            //     esp_camera_fb_return(fb);
            //     fb = NULL;
            //     if (!jpeg_converted) {
            //         res = ESP_FAIL;
            //     }
            // } else {
            FSM = eFSMImageSend;
        }
    
        vTaskDelay(10/portTICK_PERIOD_MS);

    }

        
    






}