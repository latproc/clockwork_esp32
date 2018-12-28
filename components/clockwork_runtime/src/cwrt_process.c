#include "runtime.h"
#include "rtscheduler.h"
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#define DEBUG_LOG 0
#if DEBUG_LOG
static const char* TAG = "process";
#endif

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
            // update all machine's TIMER value (TODO: optimise this) for potential use in auto states
            uint64_t up_time = upTime();
            m = getMachineIterator();
            while (m) {
                m->TIMER = up_time - m->START;
                m = nextMachine(m);
            }
            // process runnable machines
            while ( (m = nextRunnable()) != 0 ) {
                if (m) {
#if DEBUG_LOG
                    ESP_LOGI(TAG,"%lld running machine [%d] %s", upTime(), m->id, m->name);
#endif
                    struct ActionListItem *next_action;
                    next_action = list_top(&m->actions, struct ActionListItem, list);
                    if (m->executing(m)) {
                        m->execute(m, &(m->ctx) );
                        if (!m->executing(m)) {
                            NotifyDependents(m); // finished execution
                        }
                        markPending(m);
                    }
                    else if (next_action) {
                        next_action = list_pop(&m->actions, struct ActionListItem, list);
                        m->execute = next_action->action;
                        free(next_action);
                        m->execute(m, &(m->ctx) );
                        if (!m->executing(m)) {
#if DEBUG_LOG
                            ESP_LOGI(TAG,"notifying dependents of %s", m->name);
#endif
                            NotifyDependents(m); // finished execution
                        }
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
#if DEBUG_LOG
                        ESP_LOGI(TAG,"%lld changing state of %s", upTime(), m->name);
#endif

                        struct MachineListItem *item, *next;
                        list_for_each_safe(&m->depends, item, next, list) {
                            if (item->machine->handle) 
                                item->machine->handle(item->machine, m, m->state);
                        }
                    }
#if DEBUG_LOG
                    else
                        ESP_LOGI(TAG,"%lld nothing to do for %s", upTime(), m->name);
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

