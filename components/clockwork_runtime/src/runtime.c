#include <freertos/FreeRTOS.h>
#include "runtime.h"
#include <esp_log.h>
#include "mqtt_client.h"

static const char* TAG = "Runtime";

MachineBase *first_machine = 0;
static unsigned int next_id = 1;

SemaphoreHandle_t scheduler_sem = 0;
SemaphoreHandle_t process_sem = 0;
SemaphoreHandle_t io_interface_sem = 0;
SemaphoreHandle_t runtime_mutex = 0;

struct list_head global_messages;
struct list_head command_messages;

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

const char *name_from_id();

int haveMQTT() {
    return mqtt_is_connected;
}

void setMQTTstate(int which) {
    mqtt_is_connected = which;
}

extern esp_mqtt_client_handle_t client;

void publish_MQTT(MachineBase *m, int state) {
    if (!haveMQTT()) return;
    char buf[40];
    char data[40];
    snprintf(buf, 40, "/sampler/%s", m->name);
    snprintf(data, 40, "%lld %s %s", upTime(), m->name, name_from_id(state));
    esp_mqtt_client_publish(client, buf, data, 0, 0, 0);
}

void publish_MQTT_property(MachineBase *m, const char *name, int value) {
    if (!haveMQTT()) return;
    char buf[60];
    char data[40];
    snprintf(buf, 60, "/sampler/%s/%s", m->name, name);
    snprintf(data, 40, "%lld %s %s %d", upTime(), m->name, name, value);
    esp_mqtt_client_publish(client, buf, data, 0, 0, 0);
}

void sendMQTT(const char *topic, const char *data) {
    if (!haveMQTT()) return;
    esp_mqtt_client_publish(client, topic, data, 0, 0, 0);
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
    list_head_init(&global_messages);
    list_head_init(&command_messages);
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
    assert(runtime_mutex);
    BaseType_t res = xSemaphoreTakeRecursive(runtime_mutex,10);
    assert(res == pdPASS);
    cwTask *t = (cwTask *)malloc(sizeof(cwTask));;
    t->machine = m; t->f = 0;
	t->next = pending_tasks;
	pending_tasks = t;
    res = xSemaphoreGiveRecursive(runtime_mutex);
    assert(res == pdPASS);
}

void activatePending() {
    while (pending_tasks) {
        //ESP_LOGI(TAG,"%lld marking pending machine [%d] as runnable", upTime(), pending_tasks->machine->id);
        markRunnable(pending_tasks->machine);
        cwTask *t = pending_tasks;
        pending_tasks = pending_tasks->next;
        free(t);
    }
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
    publish_MQTT_property(m, name, v);
    cw_send(m, 0, cw_message_property_change);
}

void initMachineBase(MachineBase *m, const char *name) {
	m->p_next = first_machine;
    list_head_init(&m->depends);
    list_head_init(&m->actions);
    list_head_init(&m->messages);
    m->name = name;
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
        publish_MQTT(m, new_state);
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
