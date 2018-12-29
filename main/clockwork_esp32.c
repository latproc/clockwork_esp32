#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_freertos_hooks.h"
#include <stdio.h>
#include "rtscheduler.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <assert.h>
#include "runtime.h"
#include <esp_log.h>
#include "iointerface.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"


// TODO: fix error handling
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void cwrt_task(void *pvParameter)
{
    configASSERT(configUSE_RECURSIVE_MUTEXES);
    vTaskDelay(10);
    rt_init();
	cwrt_setup();
    unsigned long last = 0;
    while(1) {
		cwrt_process(&last);
        vTaskDelay(1);
    }
}

void app_main()
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

	xTaskCreate(&RTSchedulerTask, "sch_task", 10000, NULL, 5, NULL);
	xTaskCreate(&RTIOInterface, "i/o_task", 10000, NULL, 5, NULL);
    xTaskCreate(&cwrt_task, "cwrt_task", 10000, NULL, 5, NULL);

    // TODO: is it ok for app_main to exit?
    while (true) {
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }

}
