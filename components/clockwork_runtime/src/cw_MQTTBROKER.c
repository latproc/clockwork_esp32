#include "base_includes.h"
#include <freertos/FreeRTOS.h>
#include <inttypes.h>
#include "cw_MQTTBROKER.h"
#include "cw_MQTTSUBSCRIBER.h"

#define DEBUG_LOG 0
static const char* TAG = "MQTTBROKER";

SemaphoreHandle_t mqtt_mutex = 0;
struct list_head mqtt_messages;
esp_mqtt_client_handle_t system_client = 0;

struct MQTTMessageListItem {
	struct list_node list;
	struct cw_MQTTBROKER *broker;
	uint64_t timestamp;
	char *topic;
	char *msg;
};

void push_mqtt_message(struct cw_MQTTBROKER *broker, const char *topic, const char *msg) {
	uint64_t timestamp = upTime();
    BaseType_t res = xSemaphoreTake(mqtt_mutex, 10);
	if (res == pdPASS) {
		struct MQTTMessageListItem *item = (struct MQTTMessageListItem*) malloc(sizeof(struct MQTTMessageListItem));
		item->broker = broker;
		item->timestamp = timestamp;
		item->topic = strdup(topic);
		item->msg = strdup(msg);
		list_add_tail(&mqtt_messages, &item->list);
		res = xSemaphoreGive(mqtt_mutex);
		assert(res == pdPASS);
	}
}
static int have_mqtt_message() {
    return list_top(&mqtt_messages, struct MQTTMessageListItem, list) != 0;
}
struct MQTTMessageListItem *pop_mqtt_message() {
    if (!have_mqtt_message()) return 0;
    BaseType_t res = xSemaphoreTake(mqtt_mutex, 10);
	if (res == pdPASS) {
    	struct MQTTMessageListItem *mli = list_pop(&mqtt_messages, struct MQTTMessageListItem, list);
		res = xSemaphoreGive(mqtt_mutex);
		assert(res == pdPASS);
    	return mli;
	}
	return 0;
}

void cwMQTTTask(void *pvParameter) {
    while(1) {
        vTaskDelay(1);
		if (have_mqtt_message()) {
			struct MQTTMessageListItem *item = pop_mqtt_message();
			if (item == 0) continue;
			esp_mqtt_client_handle_t client = (item->broker) ? item->broker->client : system_client;
			esp_mqtt_client_publish(client, item->topic, item->msg, 0, 0, 0);
			free(item->topic);
			free(item->msg);
			free(item);
		}
    }
}


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    struct cw_MQTTBROKER *context = event->user_context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED: {
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED %s", context->machine.name);
			if (!haveMQTT()) {
				setMQTTclient(client);
				setMQTTstate(1);
				msg_id = esp_mqtt_client_subscribe(client, "/command", 0);
			}
			struct SubscriberListItem *item, *next;
			list_for_each_safe(&context->subscribers, item, next, list) {
				struct cw_MQTTSUBSCRIBER *subs = (struct cw_MQTTSUBSCRIBER *)item->s;
				msg_id = esp_mqtt_client_subscribe(client, subs->topic, 0);
			}
		}
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED %s", context->machine.name);
			setMQTTstate(0);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, %s msg_id=%d topic: %.*s", context->machine.name, event->msg_id, event->topic_len, event->topic);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, %s msg_id=%d", context->machine.name, event->msg_id);
            break;
        case MQTT_EVENT_DATA:
			receiveMQTT(context, event->topic, event->topic_len, event->data, event->data_len);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR %s", context->machine.name);
            break;
    }
    return ESP_OK;
}

#define state_cw_INIT 1
struct cw_MQTTBROKER_Vars {
	struct cw_MQTTBROKER *m;
	unsigned int l_INIT;
	const char *l_host;
	unsigned int *l_port;
};
struct cw_MQTTBROKER_Vars_backup {
	struct cw_MQTTBROKER  m;
	unsigned int l_INIT;
	const char *l_host;
	unsigned int l_port;
};
static void init_Vars(struct cw_MQTTBROKER *m, struct cw_MQTTBROKER_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_host = m->host;
	v->l_port = &m->port;
}
static void backup_Vars(struct cw_MQTTBROKER *m) {
	struct cw_MQTTBROKER_Vars *v = m->vars;
	struct cw_MQTTBROKER_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_host = v->l_host;
	b->l_port = *v->l_port;
}
Value *cw_MQTTBROKER_lookup(struct cw_MQTTBROKER *m, int symbol) {
	return 0;
}
MachineBase *cw_MQTTBROKER_lookup_machine(struct cw_MQTTBROKER *m, int symbol) {
	return 0;
}
void cw_MQTTBROKER_describe(struct cw_MQTTBROKER *m);
int cw_MQTTBROKER_check_state(struct cw_MQTTBROKER *m);
struct cw_MQTTBROKER *create_cw_MQTTBROKER(const char *name, const char *host, unsigned int port) {
	struct cw_MQTTBROKER *p = (struct cw_MQTTBROKER *)malloc(sizeof(struct cw_MQTTBROKER));
	Init_cw_MQTTBROKER(p, name, host, port);
	return p;
}
int cw_MQTTBROKER_INIT_enter(struct cw_MQTTBROKER *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_MQTTBROKER_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	//struct cw_MQTTBROKER *m = (struct cw_MQTTBROKER *)obj;
	markPending(obj);
	return 1;
}
void Init_cw_MQTTBROKER(struct cw_MQTTBROKER *m, const char *name, const char *host, unsigned int port) {
	initMachineBase(&m->machine, name);
	m->machine.class_name = "MQTTBROKER";
	m->host = host;
	m->port = port;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_MQTTBROKER_check_state;
	m->machine.handle = (message_func)cw_MQTTBROKER_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_MQTTBROKER_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_MQTTBROKER_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_MQTTBROKER_describe;
	m->vars = (struct cw_MQTTBROKER_Vars *)malloc(sizeof(struct cw_MQTTBROKER_Vars));
	m->backup = (struct cw_MQTTBROKER_Vars_backup *)malloc(sizeof(struct cw_MQTTBROKER_Vars_backup));
	mqtt_mutex = xSemaphoreCreateRecursiveMutex();
    assert(mqtt_mutex);
	list_head_init(&mqtt_messages);

	list_head_init(&m->subscribers);
	init_Vars(m, m->vars);
	size_t n = strlen(m->host) + 15;
	char *buf = (char*)malloc(n);
	snprintf(buf, n, "mqtt://%s:%d", m->host, m->port);
	ESP_LOGI(TAG,"%lld %s connecting to %s", upTime(), m->machine.name, buf);
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = buf,
        .event_handle = mqtt_event_handler,
		.user_context = (void *)m
        //.cert_pem = (const char *)iot_eclipse_org_pem_start,
    };
    m->client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(m->client);
	xTaskCreate(&cwMQTTTask, "cwMQTTTask", 10000, NULL, 3, NULL);

	MachineActions_add(cw_MQTTBROKER_To_MachineBase(m), (enter_func)cw_MQTTBROKER_INIT_enter);
	markPending(&m->machine);
}
MachineBase *cw_MQTTBROKER_To_MachineBase(struct cw_MQTTBROKER *p) { return &p->machine; }

int cw_MQTTBROKER_check_state(struct cw_MQTTBROKER *m) {
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_MQTTBROKER_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
void cw_MQTTBROKER_describe(struct cw_MQTTBROKER *m) {
	char buf[100];
	snprintf(buf, 100, "%s: %s:%d  Class: MQTTBROKER", m->machine.name, m->host, m->port);
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT(0, "/response", buf);
}
void register_subscriber(struct cw_MQTTSUBSCRIBER *subs) {
    struct SubscriberListItem *item = (struct SubscriberListItem*) malloc(sizeof(struct SubscriberListItem));
    item->s = subs;
	struct cw_MQTTBROKER *broker = (struct cw_MQTTBROKER *)subs->_broker;
    list_add_tail(&broker->subscribers, &item->list);
}
