#ifndef __cw_MQTTPUBLISHER_h__
#define __cw_MQTTPUBLISHER_h__

#include "runtime.h"
#define Value int
struct cw_MQTTPUBLISHER_Vars;
struct cw_MQTTPUBLISHER_Vars_backup;
struct cw_MQTTPUBLISHER {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_broker;
	const char *topic;
	Value message;
	Value VALUE;
	struct cw_MQTTPUBLISHER_Vars *vars;
	struct cw_MQTTPUBLISHER_Vars_backup *backup;
};
struct IOAddress *cw_MQTTPUBLISHER_getAddress(struct cw_MQTTPUBLISHER *p);
struct cw_MQTTPUBLISHER *create_cw_MQTTPUBLISHER(const char *name, MachineBase *broker, const char *topic, Value message);
void Init_cw_MQTTPUBLISHER(struct cw_MQTTPUBLISHER * , const char *name, MachineBase *broker, const char *topic, Value message);
MachineBase *cw_MQTTPUBLISHER_To_MachineBase(struct cw_MQTTPUBLISHER *);
#endif
