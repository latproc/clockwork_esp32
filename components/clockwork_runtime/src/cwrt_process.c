#include "runtime.h"
#include "rtscheduler.h"
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#define DEBUG_LOG 0

static const char* TAG = "process";

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
                    struct ActionListItem *next_action;
                    next_action = list_top(&m->actions, struct ActionListItem, list);
                    if (m->executing(m)) {
                        m->execute(m, &(m->ctx) );
                        markPending(m);
                    }
                    else if (next_action) {
#if DEBUG_LOG
                        ESP_LOGI(TAG,"starting new action on %s", m->name);
#endif
                        next_action = list_pop(&m->actions, struct ActionListItem, list);
                        m->execute = next_action->action;
                        free(next_action);
                        m->execute(m, &(m->ctx) );
                        markPending(m);
                    }
                    else if (list_top(&m->messages, struct MessageListItem, list)) {
                        struct MessageListItem *item;
                        item = list_pop(&m->messages, struct MessageListItem, list);
                        while (item) {
                            if (m->handle) {
#if DEBUG_LOG
                                ESP_LOGI(TAG,"%lld sending [%d] to [%d]", upTime(), item->message, m->id);
#endif
                                m->handle(m, item->from, item->message);
                            }
                            free(item);
                            item = list_pop(&m->messages, struct MessageListItem, list);
                        }
                    }
                    else if (m->check_state && m->check_state(m)) { // state change
                        struct MachineListItem *item, *next;
                        list_for_each_safe(&m->depends, item, next, list) {
                            if (item->machine->handle) 
                                item->machine->handle(item->machine, m, m->state);
                        }
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

