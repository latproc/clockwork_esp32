#ifndef __cw_MQTTSUBSCRIBER_h__
#define __cw_MQTTSUBSCRIBER_h__

#include "runtime.h"
#define Value int
struct cw_MQTTSUBSCRIBER_Vars;
struct cw_MQTTSUBSCRIBER_Vars_backup;
struct cw_MQTTSUBSCRIBER {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_broker;
	const char *topic;
	Value message; // 
	Value VALUE;
	struct cw_MQTTSUBSCRIBER_Vars *vars;
	struct cw_MQTTSUBSCRIBER_Vars_backup *backup;
};

struct SubscriberListItem {
    struct list_node list;
	struct cw_MQTTSUBSCRIBER *s;
};

struct IOAddress *cw_MQTTSUBSCRIBER_getAddress(struct cw_MQTTSUBSCRIBER *p);
struct cw_MQTTSUBSCRIBER *create_cw_MQTTSUBSCRIBER(const char *name, MachineBase *broker, const char *topic);
void Init_cw_MQTTSUBSCRIBER(struct cw_MQTTSUBSCRIBER * , const char *name, MachineBase *broker, const char *topic);
MachineBase *cw_MQTTSUBSCRIBER_To_MachineBase(struct cw_MQTTSUBSCRIBER *);
#endif
