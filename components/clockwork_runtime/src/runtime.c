#include <freertos/FreeRTOS.h>
#include "runtime.h"
#include <esp_log.h>
#include "cw_MQTTBROKER.h"
#include "cw_MQTTSUBSCRIBER.h"
#include "mqtt_client.h"

static const char* TAG = "Runtime";

MachineBase *first_machine = 0;
static unsigned int next_id = 1;

SemaphoreHandle_t scheduler_sem = 0;
SemaphoreHandle_t process_sem = 0;
SemaphoreHandle_t io_interface_sem = 0;
SemaphoreHandle_t runtime_mutex = 0;
SemaphoreHandle_t message_mutex = 0;

struct list_head global_messages;
struct list_head command_messages;
struct list_head external_messages;

int network_is_connected = 0;
int mqtt_is_connected = 0;

void debug(const char *s) {
   ESP_LOGI(TAG,"%lld %s", upTime(), s);
}

void debugInt(const char *s, int i) {
  ESP_LOGI(TAG,"%lld %s%d", upTime(), s, i);
}

uint64_t upTime() {
    return (uint64_t)esp_timer_get_time() / 1000;
}

int haveMQTT() {
    return mqtt_is_connected;
}

void setMQTTclient(esp_mqtt_client_handle_t clt) {
    system_client = clt;
}


void setMQTTstate(int which) {
    mqtt_is_connected = which;
}

#if 0
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED main");
            setMQTTstate(1);
            msg_id = esp_mqtt_client_subscribe(client, "/command", 0);
            ESP_LOGI(TAG, "sent subscribe successful (/command), msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED main");
            setMQTTstate(0);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, main msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, main msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, main msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: {
            char *cmd = malloc(event->data_len+1);
            memcpy(cmd, event->data, event->data_len);
            cmd[event->data_len] = 0;
            ESP_LOGI(TAG, "MQTT_EVENT_DATA %s", cmd);
            //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //printf("DATA=%.*s\r\n", event->data_len, event->data);
            push_command(cmd);
        }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR main");
            break;
    }
    return ESP_OK;
}
#endif

struct ExternalMessageItem {
	struct list_node list;
    struct cw_MQTTSUBSCRIBER *s;
    int message;
};

static void push_message(struct cw_MQTTSUBSCRIBER *s, int message) {
    BaseType_t res = xSemaphoreTakeRecursive(message_mutex,10);
    if (res == pdPASS) {
        //ESP_LOGI(TAG, "push_message got message_mutex");
        struct ExternalMessageItem *item = (struct ExternalMessageItem*) malloc(sizeof(struct ExternalMessageItem));
        item->s = s;
        item->message = message;
        list_add_tail(&external_messages, &item->list);
        res = xSemaphoreGiveRecursive(message_mutex);
        assert(res == pdPASS);
        //ESP_LOGI(TAG, "push_message released message_mutex");
    }
    else {
        //ESP_LOGI(TAG, "failed to get mutex to push message");
    }
    taskYIELD();
}
int have_external_message() {
    return list_top(&external_messages, struct ExternalMessageItem, list) != 0;
}
void process_next_external_message() {
    if (!have_external_message()) return;
    BaseType_t res = xSemaphoreTakeRecursive(message_mutex,10);
    if (res == pdPASS) {
        //ESP_LOGI(TAG, "process_next_external_message got message_mutex");
        struct ExternalMessageItem *ml = list_pop(&external_messages, struct ExternalMessageItem, list);
        res = xSemaphoreGiveRecursive(message_mutex);
        assert(res == pdPASS);
        //ESP_LOGI(TAG, "process_next_external_message released message_mutex");
        if (!ml) return;
        struct cw_MQTTSUBSCRIBER *sub = ml->s;
        int message = ml->message;
        free(ml);
        BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
        if (res == pdPASS) {

            sub->machine.set_value(&sub->machine, "message", &sub->message, message);
            NotifyDependents(&sub->machine);
            markPending(&sub->machine);
            res = xSemaphoreGiveRecursive(runtime_mutex);
            assert(res == pdPASS);
        }
    }
    else {
        //ESP_LOGI(TAG, "failed to get mutex to process next message");
    }
}

void receiveMQTT(struct cw_MQTTBROKER *context, const char *topic, size_t topic_len, const char *data, size_t data_len) {
    if (!context) return;
    //ESP_LOGI(TAG, "MQTT_EVENT_DATA TOPIC=%.*s DATA=%.*s", topic_len, topic, data_len, data);
    if (strncmp(topic, "/command", topic_len) == 0) {
        char *cmd = malloc(data_len+1);
        memcpy(cmd, data, data_len);
        cmd[data_len] = 0;
        push_command(cmd);
    }
    else {
        char buf[data_len+1];
        memcpy(buf, data, data_len);
        buf[data_len] = 0;
        char *p;
        long message = strtol(buf, &p, 10);
        struct SubscriberListItem *item, *next;
        list_for_each_safe(&context->subscribers, item, next, list) {
            if (item->s && strncmp(item->s->topic, topic, topic_len) == 0) {
                push_message(item->s, message);
                break;
            }
        }
    }
}

void publish_MQTT(struct cw_MQTTBROKER *broker, MachineBase *m, int state) {
    BaseType_t res = xSemaphoreTakeRecursive(message_mutex,10);
    if (res == pdPASS) {
        //ESP_LOGI(TAG, "publish_MQTT got message_mutex");
        if (haveMQTT()) {
            char buf[40];
            char data[40];
            snprintf(buf, 40, "/sampler/%s", m->name);
            snprintf(data, 40, "%lld %s %s", upTime(), m->name, name_from_id(state));
            push_mqtt_message(broker, buf, data);
        }
        res = xSemaphoreGiveRecursive(message_mutex);
        assert(res == pdPASS);
        //ESP_LOGI(TAG, "publish_MQTT released message_mutex");
    }
    else {
        //ESP_LOGI(TAG, "publish_MQTT failed to get message mutex");
    }
}

void publish_MQTT_property(struct cw_MQTTBROKER *broker, MachineBase *m, const char *name, int value) {
   BaseType_t res = xSemaphoreTakeRecursive(message_mutex,10);
    if (res == pdPASS) {
        //ESP_LOGI(TAG, "publish_MQTT_property got message_mutex");
        //ESP_LOGI(TAG,"%lld publish_MQTT_property", upTime());
        if (!haveMQTT()) goto done_publish_property;
        char buf[60];
        char data[40];
        snprintf(buf, 60, "/sampler/%s/%s", m->name, name);
        snprintf(data, 40, "%lld %s %s %d", upTime(), m->name, name, value);
        //ESP_LOGI(TAG,"%lld publishing %s/%s", upTime(),buf, data);
        push_mqtt_message(broker, buf, data);
    done_publish_property:
        res = xSemaphoreGiveRecursive(message_mutex);
        assert(res == pdPASS);
        //ESP_LOGI(TAG, "publish_MQTT_property released message_mutex");
    }
    else {
        //ESP_LOGI(TAG, "publish_MQTT_property failed to get message mutex");
    }
}

void sendMQTT(struct cw_MQTTBROKER *broker, const char *topic, const char *data) {
   BaseType_t res = xSemaphoreTakeRecursive(message_mutex,10);
    if (res == pdPASS) {
        //ESP_LOGI(TAG, "sendMQTT got message_mutex");
        if (haveMQTT())
            push_mqtt_message(broker, topic, data);


        res = xSemaphoreGiveRecursive(message_mutex);
        assert(res == pdPASS);
        //ESP_LOGI(TAG, "sendMQTT released message_mutex");
    }
    else {
        //ESP_LOGI(TAG, "sendMQTT failed to get message mutex");
    }
}

void rt_init(void) {
    assert(!scheduler_sem); // make sure setup() has not already been called
    scheduler_sem = xSemaphoreCreateBinary();
    assert(scheduler_sem);
    process_sem = xSemaphoreCreateBinary();
    assert(process_sem);
    io_interface_sem = xSemaphoreCreateBinary();
    assert(io_interface_sem);
    runtime_mutex = xSemaphoreCreateRecursiveMutex();
    assert(runtime_mutex);
    message_mutex = xSemaphoreCreateRecursiveMutex();
    list_head_init(&global_messages);
    list_head_init(&command_messages);
    list_head_init(&external_messages);

    //const esp_mqtt_client_config_t mqtt_cfg = {
    //    .uri = CONFIG_ESP_MQTT_BROKER,
    //    .event_handle = mqtt_event_handler,
    //    //.cert_pem = (const char *)iot_eclipse_org_pem_start,
    //};
    //system_client = esp_mqtt_client_init(&mqtt_cfg);
    //esp_mqtt_client_start(system_client);

}

MachineBase *getMachineIterator() {
	return first_machine;
}

MachineBase *nextMachine(MachineBase *m) {
	if (!m) return first_machine;
	return m->p_next;
}

/* run queue */

typedef void(*task_func)(MachineBase *machine);

typedef struct cwTask {
	struct cwTask *next;
	MachineBase *machine;
    task_func f;
} cwTask;

cwTask *run_list = 0;
cwTask *state_check_list = 0;
cwTask *current_task = 0;
cwTask *to_clear = 0;
cwTask *pending_tasks = 0;

cwTask *pushTask(cwTask **l, cwTask *t) {
    t->next = *l; *l = t; return t;
}

void markRunnable(MachineBase *m) {
    //ESP_LOGI(TAG,"%lld marking machine [%d] as runnable\n", upTime(), m->id);
    assert(runtime_mutex);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
    assert(res == pdPASS);
	cwTask *t = run_list;
    cwTask *last = 0;
	while (t) {
		if (t->machine == m) goto done_markRunnable;
        last = t;
		t = t->next;
	}
    t = (cwTask *)malloc(sizeof(cwTask));
    t->machine = m; t->f = 0;
	if (last) { t->next = 0; last->next = t; } else { t->next = run_list; run_list = t; }

done_markRunnable:
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
}

void markPending(MachineBase *m) {
    //ESP_LOGI(TAG,"%lld marking machine [%d] as pending", upTime(), m->id);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,2);
    while (res != pdPASS) {
        ESP_LOGI("runtime","%lld marking machine [%s] as pending (trouble getting runtime semaphore", upTime(), m->name);
        res = xSemaphoreTakeRecursive(runtime_mutex,2);
        vTaskDelay(1);
    }
	cwTask *t = pending_tasks;
	while (t) {
		if (t->machine == m) goto done_markPending;
		t = t->next;
	}
    t = (cwTask *)malloc(sizeof(cwTask));;
    t->machine = m; t->f = 0;
	t->next = pending_tasks;
	pending_tasks = t;
done_markPending:
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
}

void activatePending() {
    assert(runtime_mutex);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
    while (pending_tasks) {
        //ESP_LOGI(TAG,"%lld marking pending machine [%d] as runnable", upTime(), pending_tasks->machine->id);
        markRunnable(pending_tasks->machine);
        cwTask *t = pending_tasks;
        pending_tasks = pending_tasks->next;
        free(t);
    }
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
}

cwTask *clearTaskList(cwTask *l) {
	cwTask *t = l;
	while (l) {
		l = l->next;
	    free(t);
		t = l;
	}
    return l;
}

/* returns the next task in the current run list.
	The run list is automatically reset so that entries that get marked as runnable can be re-added
*/
cwTask *popTask() {
    current_task = run_list;
    if (run_list) run_list = run_list->next;
    return current_task;
}

struct CommandListItem {
	struct list_node list;
	char *command;
};

void push_command(char *cmd) {
    struct CommandListItem *item = (struct CommandListItem*) malloc(sizeof(struct CommandListItem));
    item->command = cmd;
    list_add_tail(&command_messages, &item->list);
}
int have_command() {
    return list_top(&command_messages, struct CommandListItem, list) != 0;
}
char *pop_command() {
    if (!have_command()) return 0;
    struct CommandListItem *cl = list_pop(&command_messages, struct CommandListItem, list);
    if (!cl) return 0;
    char *cmd = cl->command;
    free(cl);
    return cmd;
}

MachineBase *nextRunnable() {
    assert(runtime_mutex);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
    assert(res == pdPASS);
	cwTask *t = popTask();
	to_clear = clearTaskList(to_clear);
	if (t) {
		MachineBase *m = t->machine;
		t->next = to_clear; to_clear = t;
        res = xSemaphoreGiveRecursive(runtime_mutex);
        assert(res == pdPASS);
		return m;
	}
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
	return 0;
}

int runnableMachines() { return run_list != 0; }

int stateCheckMachines() { return state_check_list != 0; }

cwTask *popStateCheck() {
    current_task = state_check_list;
    if (state_check_list) state_check_list = state_check_list->next;
    return current_task;
}

void markStateCheck(MachineBase *m) {
    assert(runtime_mutex);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
    assert(res == pdPASS);
    cwTask *t = (cwTask *)malloc(sizeof(cwTask));;
    t->machine = m; t->f = 0;
	t->next = state_check_list;
	state_check_list = t;
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
}

MachineBase *nextStateCheck() {
    assert(runtime_mutex);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
    assert(res == pdPASS);
	cwTask *t = popStateCheck();
	to_clear = clearTaskList(to_clear);
	if (t) {
		MachineBase *m = t->machine;
		t->next = to_clear; to_clear = t;
        res = xSemaphoreGiveRecursive(runtime_mutex);
        assert(res == pdPASS);
		return m;
	}
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
	return 0;
}

int noExecutionStage(MachineBase *m) { return 0; }
int stateExecutionStage(MachineBase *m) { return m->execute != 0; }
int noAutoStateChanges(MachineBase*m) { return 0; }

void default_set_value(MachineBase *m, const char *name, int *p, int v) {
    *p = v;
    publish_MQTT_property(0, m, name, v);
    cw_send(m, 0, cw_message_property_change);
}

void default_describe(MachineBase *m) {
	char buf[100];
	snprintf(buf, 100, "%s: %s  Class: %s", m->name, name_from_id(m->state), m->class_name);
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->TIMER);
	sendMQTT(0,"/response", buf);
}

void initMachineBase(MachineBase *m, const char *name) {
	m->p_next = first_machine;
    list_head_init(&m->depends);
    list_head_init(&m->actions);
    list_head_init(&m->messages);
    m->name = name;
    m->class_name = "unknown";
	first_machine = m;
    m->id = next_id++;
	m->flags = FLAG_PASSIVE;
	m->START = upTime();
	m->TIMER = 0;
	m->state = state_INIT;
	m->init = 0;
    m->ctx = 0;
    
	m->executing = stateExecutionStage;
    m->execute = 0;
    m->handle = 0;
    m->lookup = 0;
    m->lookup_machine = 0;
    m->describe = 0;
    m->set_value = default_set_value;
    m->describe = default_describe;
	m->check_state = noAutoStateChanges;
}

void resetMachineBase(MachineBase *m) {
	m->START = upTime();
	m->TIMER = 0;
	m->state = state_INIT;
}

void changeMachineState(struct MachineBase *m, int new_state, enter_func handler) {
    if (m->execute != 0) {
        ESP_LOGI(TAG,"%lld execute is still set in %s check_state", upTime(), m->name);
    }
    assert(m->execute == 0);
    if (m->state != new_state) {
        m->state = new_state;
        m->TIMER = 0;
        m->START = upTime();
        m->execute = handler;
        publish_MQTT(0, m, new_state);
    }
}

void goto_sleep(MachineBase *m) {
    m->flags |= FLAG_SLEEPING;
}

void wake_up(MachineBase *m) {
    m->flags &= MASK_SLEEPING;
}

int is_asleep(MachineBase *m) {
    return m->flags & FLAG_SLEEPING;
}

void MachineDependencies_add(struct MachineBase *machine, struct MachineBase *dependent) {
    struct MachineListItem *item = (struct MachineListItem *)malloc(sizeof(struct MachineListItem));
    item->machine = dependent;
    list_add(&machine->depends, &item->list);
    ESP_LOGI(TAG,"%lld %s has dependent %s", upTime(), machine->name, dependent->name);
}

void MachineActions_add(struct MachineBase *machine, enter_func f) {
    struct ActionListItem *item = (struct ActionListItem *)malloc(sizeof(struct ActionListItem));
    item->action = f;
    list_add_tail(&machine->actions, &item->list);
    markPending(machine);
}

void NotifyDependents_state_change(struct MachineBase *machine, int state) {
    struct MachineListItem *item, *next;
    list_for_each_safe(&machine->depends, item, next, list) {
        if (item->machine) { 
          cw_send(item->machine, machine, state);
        }
    }
}

void NotifyDependents(struct MachineBase *machine) {
    struct MachineListItem *item, *next;
    list_for_each_safe(&machine->depends, item, next, list) {
        if (item->machine) {
            markPending(item->machine);
        }
    }
}

void cw_send(struct MachineBase *to, struct MachineBase *from, int message) {
    struct MessageListItem *item = (struct MessageListItem *)malloc(sizeof(struct MessageListItem));
    item->from = from;
    item->message = message;
    if (to) {
        list_add_tail(&to->messages, &item->list);
        markPending(to);
    }
    else
        list_add_tail(&global_messages, &item->list);
}
