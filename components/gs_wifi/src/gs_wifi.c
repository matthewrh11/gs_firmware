#include "gs_wifi.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "string.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}


void gs_wifi_init() {
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
}

void gs_wifi_connect(char* SSID, char* KEY){

    wifi_config_t sta_config = { 0 };

    sta_config.sta.bssid_set = false;

    memcpy(sta_config.sta.ssid, SSID, sizeof(sta_config.sta.ssid));
    memcpy(sta_config.sta.password, KEY, sizeof(sta_config.sta.password));

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_LOGE("wifi check", "%d", esp_wifi_connect());

}
