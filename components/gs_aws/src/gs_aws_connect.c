#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

const char* DEVICE_CERT =
"-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUQNvXMOYfYq9L6GY0t4cCQrEjbBswDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIxMDMwNTE2Mjcy\n"
"MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMPHrsGhKaQzUozcFgXS\n"
"6iAQMgM6GOXnWsEwt88U/WUXPwYOQAxdLjlc8kQhBHVDRF4TNGRDRlTkQSTzvGOS\n"
"kY52XyDab6ARM5AEp8GLPsSzvquKWdUH5jPD0L8SE2nIchQ+CmfvMmN17dDE790b\n"
"j6+1iqhwFST7V/OBZV3tYlCvhuA/9TU+DDj9BHbg4y7plkpx6sn2OQEHq7QdCjN+\n"
"K+nTSGuApg4/3GobZQTs1Xh4R1asvuEfpHiweqX8QoKwTzx4rXR6EN5X+RaQonw3\n"
"g5ZYensGsMqZFqzNrTp8irdvwCW81xNlfhGpZh3y9zwzznLNKuKxGvOq05sXtcVK\n"
"P4MCAwEAAaNgMF4wHwYDVR0jBBgwFoAU/hhdfxuPJcR+I7FThdheop28ERIwHQYD\n"
"VR0OBBYEFIE4ec1M4iitQFNgfM5dbgbRvYswMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCqRkNQ/AsU+UYIgFeSwzyVM59e\n"
"CDXD1tT7DpsgXOwLsNef35yDp8qBPKNotFNy+7Ck1tCtnNqqQ5bgfiaCxNNzKmg8\n"
"N3jFVh80AATtqRLPu44UB5U9hdP5GgtjuhWX3SOcFB88mVj/gKSYTZDo3OPItNtX\n"
"fenUol3Cv6whtq5m69HeFzwGZmT9EcATE6UZEiX1svRsmEVfXifCbmBs3kFCrco1\n"
"XFTR47ilMnWFk+rES0TGCAZ4XXjV47xgHum4xBVnE6fmo4x+jYgTW62plXl9ls3d\n"
"0nAL6fladAj1MIRjjno8jx5/vlDpviYjy/glwi0ATJRx0drialAOVDWHhY+Y\n"
"-----END CERTIFICATE-----";

const char* ROOT_CA =
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----";

const char* PRIV_KEY =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAw8euwaEppDNSjNwWBdLqIBAyAzoY5edawTC3zxT9ZRc/Bg5A\n"
"DF0uOVzyRCEEdUNEXhM0ZENGVORBJPO8Y5KRjnZfINpvoBEzkASnwYs+xLO+q4pZ\n"
"1QfmM8PQvxITachyFD4KZ+8yY3Xt0MTv3RuPr7WKqHAVJPtX84FlXe1iUK+G4D/1\n"
"NT4MOP0EduDjLumWSnHqyfY5AQertB0KM34r6dNIa4CmDj/cahtlBOzVeHhHVqy+\n"
"4R+keLB6pfxCgrBPPHitdHoQ3lf5FpCifDeDllh6ewawypkWrM2tOnyKt2/AJbzX\n"
"E2V+EalmHfL3PDPOcs0q4rEa86rTmxe1xUo/gwIDAQABAoIBADPamZCzbOIOrdid\n"
"GkAkotonL/fm1V1fL4xnqOdyOcMiqxChiZk8vNd4pLoivJmKXXtFPuUnZGNrqU1u\n"
"OQFueprvAzciY9pTdAK7GOuMQ+25ZdDtbemck/416P4SfY3RB1YRMJRiqEfrnjdj\n"
"UdyQRjhVrwn51b/bymm2ZfDAxP91ZqUlcfW1wjghoxqJ256PNGLOtcFYWxiJbcVT\n"
"ngdcQRBzPIDM4Zfd9ZTeaJWx61+i0Q6J0/xCOZMdQJ9C1ZTIcvBa5U1PAPy7rhKl\n"
"XeI4oK5bZ2MnGiAYYrvsq2e2iIIEUEq5ywoIPIJyTt/EViYqMDhMdiRCgaunxcGY\n"
"TE9ymvECgYEA/E4h3V8ygQ8l4kHJS/Rs3NT1UsOELw43zu/ghtMYWd0wibngHxdS\n"
"Pg94H9Cs3NqZfFeLmFvzoonnbEVR37/YXlIkwda3QzxIByiZw/WWMfjsLgA9j8oY\n"
"+wZuxFm9bKPg1upx/GWGcZl3uGrjkdtawOPgfVnunXHxmzZcEXWeEV0CgYEAxqWk\n"
"lK191/HFCLhHQTXnMVyp+wtH//crccWLEmP9/JKpz1vcQHT5KOqUrXj/rrwK2bmQ\n"
"UKPQzo9DeHB70/HcR/oj70VTcxy6c8w6hsAce5p43jarymHzA5sNiexPYayaeBjl\n"
"80CY9QJ9s3M9y5OEkcEeE/cCDmQeBLmX89ZmJl8CgYB7CUL8aAsB4rgvsLMQJh7c\n"
"RcFoXc3dAIr4kAOm29Q5JmMgFpOTtdS2TObgZOuXO8azWyHI57ZUoflH1DoL6csC\n"
"m/4uxJ1T3Vamnwb1B5L5xZCvEhDzSLOL3mQvvi9ghj4DsApr8bpxNUyrkvxsyVJl\n"
"U/rKRgvVnZYbgZO+M+QkRQKBgQCJPZuNVKei79gvBBHGjdUil2CgntSplTygp9hD\n"
"Zm3td+514oMyzR3vFNKPXw9Wy3e7oKaB8AutCfGVXjFO5sRHvhvhjUXwO6LdqfWt\n"
"b0C+bKSg8aiSdEWO8irSZT8avkMiQiFsg+6pJ4Vs22uQIGTwxjlflcIH60eKACdp\n"
"QXDlhQKBgQCSxj1iqL198E0jS+N7I6t4xPegKbidFtB25mIXtcMvP9M+lIS7oEgA\n"
"OroQ6/vlsWTUW4Djs//8222HfAuqLq9Y/4Q0EJegnxpHrbKSJVJGH4gnAU/BQ8ZE\n"
"mqkFpsbzPkXqGtIlEkJPmg3k34B5UEhfe+VMBt/gzZA5yDLFhXYOqQ==\n"
"-----END RSA PRIVATE KEY-----";

const char* TOPIC = "topics/BankInfoStatus";
const int CONNECTED_BIT = BIT0;
static const char *TAG = "gs_aws";
const char* CLIENT_ID = "Main_Pedal";

/*
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/**
* @brief Default MQTT port is pulled from the aws_iot_config.h
*/
uint32_t port = AWS_IOT_MQTT_PORT;

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    //ESP_LOGI(TAG, "Subscribe callback");
    ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    ESP_LOGW(TAG, "MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) {
        return;
    }

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            ESP_LOGW(TAG, "Manual Reconnect Successful");
        } else {
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
        }
    }
}

void aws_iot_task(void *param) {
    char cPayload[100];

    int32_t i = 0;

    IoT_Error_t rc = FAILURE;

    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IoT_Publish_Message_Params paramsQOS0;
    IoT_Publish_Message_Params paramsQOS1;

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = "a1lyjy5lwlbc46-ats.iot.us-east-1.amazonaws.com";
    mqttInitParams.port = 443;
    mqttInitParams.pRootCALocation = ROOT_CA;
    mqttInitParams.pDeviceCertLocation = DEVICE_CERT;
    mqttInitParams.pDevicePrivateKeyLocation = PRIV_KEY;
    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
        abort();
    }

    connectParams.keepAliveIntervalInSec = 10;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    /* Client ID is set in the menuconfig of the example */
    connectParams.pClientID = CLIENT_ID;
    connectParams.clientIDLen = (uint16_t) strlen(CLIENT_ID);
    connectParams.isWillMsgPresent = false;

    ESP_LOGI(TAG, "Connecting to AWS...");
    do {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if(SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while(SUCCESS != rc);

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        abort();
    }

    const int TOPIC_LEN = strlen(TOPIC);

    ESP_LOGI(TAG, "Subscribing to %s", TOPIC);
    rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    }

    sprintf(cPayload, "%s : %d ", "hello from SDK", i);

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *) cPayload;
    paramsQOS0.isRetained = 0;

    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) cPayload;
    paramsQOS1.isRetained = 0;

    while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

        //Max time the yield function will wait for read messages
        rc = aws_iot_mqtt_yield(&client, 100);
        if(NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

//        //ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
//        vTaskDelay(1000 / portTICK_RATE_MS);
//        sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS0)", i++);
//        paramsQOS0.payloadLen = strlen(cPayload);
//        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS0);
//
//        //sprintf(cPayload, "%s : %d ", "hello from ESP32 (QOS1)", i++);
//        paramsQOS1.payloadLen = strlen(cPayload);
//        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);
//        if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
//            ESP_LOGW(TAG, "QOS1 publish ack not received.");
//            rc = SUCCESS;
//        }
    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");
    abort();
}


