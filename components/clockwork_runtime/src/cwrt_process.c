#include "runtime.h"
#include "rtscheduler.h"
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#define DEBUG_LOG 0
static const char* TAG = "process";

int split_params(char *cmd, const char *params[], int n) {
    int i = 0;
    params[i++] = cmd;
    char *p = strchr(cmd, ' ');
    while (p && *p) {
        *p++ = 0; 
        while (*p == ' ') ++p;
        params[i++] = p;
        p = strchr(p, ' ');
        if (i>=n-1) break;
    }
    params[i] = 0;
    return i;
}

void update_timers() {
    // update all machine's TIMER value 
    uint64_t up_time = upTime();
    MachineBase *m = getMachineIterator();
    while (m) {
        m->TIMER = up_time - m->START;
        m = nextMachine(m);
    }
}

void cwrt_process(unsigned long *last) {
    MachineBase *m = 0;

    if (runnableMachines() || stateCheckMachines() || have_command() || have_external_message())
        update_timers();

    if (runnableMachines())
    {
#if DEBUG_LOG
        ESP_LOGI(TAG,"%lld running machines", upTime());
#endif
        // process runnable machines
        while ( (m = nextRunnable()) != 0 ) {
#if DEBUG_LOG
            ESP_LOGI(TAG,"%lld running machine [%d] %s", upTime(), m->id, m->name);
#endif
            struct ActionListItem *next_action;
            next_action = list_top(&m->actions, struct ActionListItem, list);
            if (m->executing(m) || next_action) {
                wake_up(m);
                while (m->executing(m) || next_action) {
#if DEBUG_LOG
                    ESP_LOGI(TAG,"%lld executing action in [%d] %s", upTime(), m->id, m->name);
#endif
                    if (m->executing(m) && !m->execute(m, &(m->ctx))){
#if DEBUG_LOG
                        ESP_LOGI(TAG,"%lld action did not complete in [%d] %s", upTime(), m->id, m->name);
#endif
                        // if the execute failed (returned 0) and is still executing, move on to the next machine
                        if (m->executing(m)) {
                            if (!is_asleep(m))
                                markPending(m); // this machine needs more time to complete (TODO: only if it hasn't scheduled a wakeup)
                            //ESP_LOGI(TAG,"%lld machine blocked [%d] %s", upTime(), m->id, m->name);
                            goto machine_blocked;
                        }
                    }
                    else if (m->executing(m)) {
#if DEBUG_LOG
                        ESP_LOGE(TAG,"%lld after executing, machine %s did not clean up action", upTime(), m->name);
#endif
                    }
                    else if (next_action) {
#if DEBUG_LOG
                        ESP_LOGI(TAG,"%lld getting next action in [%d] %s", upTime(), m->id, m->name);
#endif
                        next_action = list_pop(&m->actions, struct ActionListItem, list);
                        m->execute = next_action->action;
                        free(next_action);
                        next_action = list_top(&m->actions, struct ActionListItem, list);
                    }
                }
                NotifyDependents(m);
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
                        m->handle(m, item->from, item->message); //TODO: do we need to do anything special here w.r.t. coroutine handling?
                    }
                    free(item);
                    item = list_pop(&m->messages, struct MessageListItem, list);
                }
            }
            else if (m->check_state) {
                markStateCheck(m);
            }
        }
        machine_blocked: ;

    }
    while ( (m = nextStateCheck()) != 0 ) {
        if (m->check_state && m->check_state(m)) { // state change
#if DEBUG_LOG
            ESP_LOGI(TAG,"%lld changing state of %s", upTime(), m->name);
#endif
            NotifyDependents_state_change(m, m->state);
        }
#if DEBUG_LOG
        else
            ESP_LOGI(TAG,"%lld nothing to do for %s", upTime(), m->name);
#endif
    }

    if (have_command()) {
        char *cmd = pop_command();
        if (cmd) {
            const char *params[8];
            split_params(cmd, params, 8);
            if (strcmp(params[0],"DESCRIBE") == 0 && params[1]) {
                MachineBase *m = getMachineIterator();
                while (m) {
                    if (strcmp(m->name, params[1]) == 0)  {
                        m->describe(m);
                        goto done_command;
                    }
                    m = nextMachine(m);
                }
                sendMQTT(0, "/response", "unknown machine");
            }
            else if (strcmp(params[0], "LIST") == 0) {
                size_t len = 0, rem = 0;
                char *response, *p;
                MachineBase *m = getMachineIterator();
                while (m) {
                    len += strlen(m->name) + 1;
                    m = nextMachine(m);
                }
                len += 2; // initial LF + terminating null
                response = (char*)malloc(len);
                p = response; rem = len;
                *p++ = '\n'; --rem; // start with a new line
                m = getMachineIterator();
                while (m) {
                    size_t n = strlen(m->name);
                    snprintf(p, rem, "%s\n", m->name);
                    ++n; // allow for the LF
                    rem -= n; p += n;
                    m = nextMachine(m);
                }
                sendMQTT(0, "/response", response);
                free(response);
           }
            else if (strcmp(params[0], "PROPERTY") == 0 && params[1] && params[2] && params[3]) {

            }
                else if (strcmp(params[0], "SEND") == 0 && params[1] && params[2] && params[3]) {

            }
            else
                sendMQTT(0, "/response", "unknown command or command format");
            done_command:
            free(cmd);
        }
    }

    {
        if (have_external_message()) { 
#if DEBUG_LOG
            ESP_LOGI(TAG,"%lld processing external messages", upTime());
#endif
            int count = 10;
            while (count-- && have_external_message())
                process_next_external_message();
#if DEBUG_LOG
            ESP_LOGI(TAG,"%lld finished external messages", upTime());
#endif
        }
    }
    activatePending();
}

