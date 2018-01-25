#include "runtime.h"
#include "rtscheduler.h"
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#define DEBUG_LOG 0

/* static const char* TAG = "process"; */

void cwrt_process(unsigned long *last) {
    MachineBase *m = 0;
    if (xSemaphoreGive(scheduler_sem) != pdFAIL) {
        BaseType_t res = xSemaphoreTake(process_sem, 1);
        while (res != pdPASS) {
            taskYIELD();
            res = xSemaphoreTake(process_sem, 1);
        }
        if (runnableMachines())
        {
#if DEBUG_LOG
            ESP_LOGI(TAG,"%lld running machines", upTime());
#endif
            while ( (m = nextRunnable()) != 0 ) {
                if (m) {
#if DEBUG_LOG
                  ESP_LOGI(TAG,"%lld running machine [%d]", upTime(), m->id);
#endif
                  m->TIMER = upTime() - m->START;
                  if (m->executing(m)) {
                      m->execute(m, &(m->ctx) );
                      markPending(m);
                  }
                  else if (m->check_state) {
                      m->check_state(m);
                  }
#if DEBUG_LOG
                  else
                      ESP_LOGI(TAG,"%lld running machine [%d]", upTime(), m->id);
#endif
                }
            }
        }
        activatePending();

        unsigned long now = upTime();
        if (now-*last > 10000) {
            debugInt("free ram:", xPortGetFreeHeapSize());
           *last = now;
        }
    }
}

