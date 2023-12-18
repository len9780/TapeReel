

/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "network_setting.h"
#include "freertos/event_groups.h"

static const char *TAG = "wifi softAP";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }else if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

bool wifi_ap(void)
{
    ESP_LOGI(TAG,"stage");
    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.ap.ssid, (char *)"softap");
    strcpy((char *)wifi_config.ap.password, (char *)"");
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.ssid_len = strlen((char *)"softap");
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.channel = 1;
    wifi_config.ap.pmf_cfg.required = true;

    if (strlen((char *)"") == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
     ESP_LOGI(TAG,"stage1");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_LOGI(TAG,"stage2");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG,"stage3");
    ESP_LOGI(TAG, "WIFI_MODE_AP started. SSID:%s password:%s channel:%d",
             (char *)"softap", (char *)"", 1);
    ESP_LOGI(TAG,"stage4");
    return ESP_OK;
}

bool wifi_sta(int timeout_ms)
{
    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.sta.ssid, (char *)"117-20");
    strcpy((char *)wifi_config.sta.password, (char *)"0978027009");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                                   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "bits=%x", bits);
    if (bits)
    {
        ESP_LOGI(TAG, "WIFI_MODE_STA connected. SSID:%s password:%s",
                 (char *)"117-20", (char *)"0978027009");
    }
    else
    {
        ESP_LOGI(TAG, "WIFI_MODE_STA can't connected. SSID:%s password:%s",
                 (char *)"117-20", (char *)"0978027009");
    }
    return (bits & CONNECTED_BIT) != 0;
}

 bool wifi_apsta(int timeout_ms)
{
    wifi_config_t ap_config = {0};
    strcpy((char *)ap_config.ap.ssid, (char *)"softap");
    strcpy((char *)ap_config.ap.password, (char *)"");
    ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    ap_config.ap.ssid_len = strlen((char *)"softap");
    ap_config.ap.max_connection = 5;
    ap_config.ap.channel = 1;

    if (strlen((char *)"") == 0)
    {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    wifi_config_t sta_config = {0};
    strcpy((char *)sta_config.sta.ssid, (char *)"117-20");
    strcpy((char *)sta_config.sta.password, (char *)"0978027009");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WIFI_MODE_AP started. SSID:%s password:%s channel:%d",
             (char *)"117-20", (char *)"", 1);

    ESP_ERROR_CHECK(esp_wifi_connect());
    int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                                   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "bits=%x", bits);
    if (bits)
    {
        ESP_LOGI(TAG, "WIFI_MODE_STA connected. SSID:%s password:%s",
                 (char *)"117-20", (char *)"0978027009");
    }
    else
    {
        ESP_LOGI(TAG, "WIFI_MODE_STA can't connected. SSID:%s password:%s",
                 (char *)"117-20", (char *)"0978027009");
    }
    return (bits & CONNECTED_BIT) != 0;
}

void wifi_init_softap(void)
{
    esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK( nvs_flash_erase() );
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);
    
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
	ESP_ERROR_CHECK( esp_wifi_start() );
}

// void app_main(void)
// {
//     // Initialize NVS
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
//     wifi_init_softap();
// }