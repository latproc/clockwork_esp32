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

//static const char* TAG = "Main";

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
	xTaskCreate(&RTSchedulerTask, "sch_task", 10000, NULL, 5, NULL);
	xTaskCreate(&RTIOInterface, "i/o_task", 10000, NULL, 5, NULL);
    xTaskCreate(&cwrt_task, "cwrt_task", 10000, NULL, 5, NULL);
}
