#ifndef MAIN_H
#define MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void CameraInit();
void WifiInit(); 
void HTTPServerStart();  


typedef struct taskConfig { 
    TaskFunction_t pvTaskCode;
    const char * const pcName;
    const configSTACK_DEPTH_TYPE uxStackDepth;
    void *pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t *pxCreatedTask;
} taskConfig;

typedef enum eNeuralNetwork { 
    eTensorFlow = 1, 
    eEspWOW, 
    ePerceptron, 
    eMulti
} eNeuralNetwork; 


typedef struct systemConfig { 
    eNeuralNetwork network; 
    uint8_t streamEnable; 
} systemConfig;



typedef enum eFiniteState { 
    eFSMCameraGet, 
    eFSMImageSend
} eFiniteState; 


extern eFiniteState FSM; 
extern size_t _jpg_buf_len;
extern uint8_t *_jpg_buf;
extern struct timeval _timestamp;
extern systemConfig mainConfig;



#endif