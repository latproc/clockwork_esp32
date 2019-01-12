#ifndef __cw_MQTTBROKER_h__
#define __cw_MQTTBROKER_h__

#include "runtime.h"
#include "rtio.h"
#include "mqtt_client.h"

void cwMQTTTask(void *pvParameter);

#define Value int
struct cw_MQTTBROKER_Vars;
struct cw_MQTTBROKER_Vars_backup;
struct cw_MQTTSUBSCRIBER;
struct cw_MQTTBROKER {
	MachineBase machine;
	struct list_head subscribers;
	const char *host;
	unsigned int port;
	esp_mqtt_client_handle_t client;
	struct cw_MQTTBROKER_Vars *vars;
	struct cw_MQTTBROKER_Vars_backup *backup;
};
struct cw_MQTTBROKER *create_cw_MQTTBROKER(const char *name, const char *host, unsigned int port);
void Init_cw_MQTTBROKER(struct cw_MQTTBROKER * , const char *name, const char *host, unsigned int port);
MachineBase *cw_MQTTBROKER_To_MachineBase(struct cw_MQTTBROKER *);
void register_subscriber(struct cw_MQTTSUBSCRIBER *);

extern esp_mqtt_client_handle_t system_client;
void push_mqtt_message(struct cw_MQTTBROKER *broker, const char *topic, const char *msg);

#endif
