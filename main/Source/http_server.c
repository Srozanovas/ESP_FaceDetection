/*HTTP serverio aptarnavimo failas
Kodas paimtas is ESP_IDF pavyzdinio kodo
GITHUB NUORODA: https://github.com/espressif/esp-idf/blob/v5.4.1/examples/protocols/http_server/simple/main/main.c*/

/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_tls_crypto.h"
#include "esp_http_server.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_check.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include "nvs_flash.h"
#include "esp_camera.h"


#include "html/pages.h"
#include "../Include/main.h"


/*Stream aptarnavimo headeriai */

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

httpd_handle_t server = NULL;
httpd_handle_t stream = NULL;


static const char *TAG = "example";


uint8_t ConfigParser(char *buf);
char *JsonBuilderConfig();

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err){
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}


/*Index handler. Pagrindinio puslapio handleris. Užkraunamas index.html failas*/

static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *)index_html, index_html_len);
};

static const httpd_uri_t main_uri = {
    .uri = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL,
};


/*Stream Handler. Paspaudus start stream atsiranda langas su vaizdu. Stream handleris apdoroja šitą langą*/

static esp_err_t stream_handler(httpd_req_t *req) {


  char *part_buf[128];



  httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");



  while (true) {

    if (FSM == eFSMImageSend){
        httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
        httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        FSM = eFSMCameraGet;
    }

    vTaskDelay(10/portTICK_PERIOD_MS);


  }
  return ESP_OK;
}


static const httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL,
};


/*Config handler parameters from web to ESP */

static esp_err_t config_handler(httpd_req_t *req) {
    char buf [100];
    char variable[32];
    char value[32];

    httpd_req_get_url_query_str(req, buf, 100); 
    ConfigParser(buf);
    printf("Got bufer: %s\r\n", buf);
    return ESP_OK;
}

httpd_uri_t cmd_uri = {
    .uri = "/control",
    .method = HTTP_GET,
    .handler = config_handler,
    .user_ctx = NULL
}; 


static esp_err_t status_handler(httpd_req_t *req) {
    
    char* json = JsonBuilderConfig(); 
    uint16_t jsonlength = 0; 
    while (json[jsonlength] != 0){ 
        jsonlength++;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, jsonlength);

    free(json);

    return ESP_OK;
}

httpd_uri_t status_uri = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = status_handler,
    .user_ctx = NULL
}; 


/*Initialise Web Server*/
void HTTPServerStart() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &main_uri);
        httpd_register_uri_handler(server, &cmd_uri);
        httpd_register_uri_handler(server, &status_uri);
   
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&stream, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering Stream URI handlers");
        httpd_register_uri_handler(stream, &stream_uri);
    
    } else {
        ESP_ERROR_CHECK(httpd_start(&stream, &config));
    }
}

void HTTPServerStop(){ 
    httpd_stop(server);
}


uint8_t ConfigParser(char *buf){ 
    uint8_t count = 0; 
    uint8_t countSafety = 0; //count for safety 
    uint8_t varNameCount = 0; 
    
    char varName[20] = {0}; 
    uint8_t varValue; 

    while(buf[count] != '='){ 
        count++; 
        countSafety++; 
        if (countSafety == 100) return 0; //Cant find = after variable 
    }
    count++;
    countSafety = 0;  
    
    while(buf[count] != ';'){ 
        varName[varNameCount] = buf[count]; 
        count++; 
        varNameCount++; 
        countSafety++; 
        if (varNameCount == 20 || countSafety == 100) return 0; //Too long name or something bad 
    }

    countSafety = 0; 
    while(buf[count] != '='){ 
        count++; 
        countSafety++; 
        if (countSafety == 100) return 0; //Cant find = after value
    }
    count++; 

    varValue = atoi((buf + count)); 

    if (!strcmp(varName, "NeuralNetwork")){ 
        mainConfig.network = varValue;
    }

    return 1;
}

char *JsonBuilderConfig(){ 
    char *strPtr = calloc(300 , sizeof(char)); //300baitu json 
    uint16_t length = 0; 
    length = sprintf(strPtr, "{"); 
    length += sprintf(strPtr+length, "\"%s\":%u", "NeuralNetwork", (uint8_t)(mainConfig.network));
    sprintf(strPtr+length, "}");
    ESP_LOGI(TAG, "%s", strPtr);
    return strPtr;
}
