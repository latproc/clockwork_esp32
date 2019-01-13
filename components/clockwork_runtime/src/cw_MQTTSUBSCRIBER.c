
#include "base_includes.h"
#include "cw_MQTTBROKER.h"
#include "cw_MQTTSUBSCRIBER.h"
#define DEBUG_LOG 0
//static const char* TAG = "MQTTSUBSCRIBER";

#define state_cw_INIT 1
struct cw_MQTTSUBSCRIBER_Vars {
	struct cw_MQTTSUBSCRIBER *m;
	unsigned int l_INIT;
	unsigned int *l_broker;
	Value *l_message;
	const char *l_topic;
};
struct cw_MQTTSUBSCRIBER_Vars_backup {
	struct cw_MQTTSUBSCRIBER  m;
	unsigned int l_INIT;
	unsigned int  l_broker;
	Value  l_message;
	const char *l_topic;
};
static void init_Vars(struct cw_MQTTSUBSCRIBER *m, struct cw_MQTTSUBSCRIBER_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_broker = &m->_broker->state;
	v->l_message = &m->message;
	v->l_topic = m->topic;
}
static void backup_Vars(struct cw_MQTTSUBSCRIBER *m) {
	struct cw_MQTTSUBSCRIBER_Vars *v = m->vars;
	struct cw_MQTTSUBSCRIBER_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_broker = *v->l_broker;
	b->l_topic = v->l_topic;
	b->l_message = *v->l_message;
}
int *cw_MQTTSUBSCRIBER_lookup(struct cw_MQTTSUBSCRIBER *m, int symbol) {
  if (symbol == sym_VALUE) return &m->VALUE;
  if (symbol == sym_message) return &m->message;
  return 0;
}
MachineBase *cw_MQTTSUBSCRIBER_lookup_machine(MachineBase *m, int symbol) {
  struct cw_MQTTSUBSCRIBER *sub = (struct cw_MQTTSUBSCRIBER*)m;
  if (symbol == sym_broker) return sub->_broker;
  return 0;
}
void cw_MQTTSUBSCRIBER_describe(struct cw_MQTTSUBSCRIBER *m);
int cw_MQTTSUBSCRIBER_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_MQTTSUBSCRIBER_check_state(struct cw_MQTTSUBSCRIBER *m);
struct cw_MQTTSUBSCRIBER *create_cw_MQTTSUBSCRIBER(const char *name, MachineBase *broker, const char *topic) {
	struct cw_MQTTSUBSCRIBER *p = (struct cw_MQTTSUBSCRIBER *)malloc(sizeof(struct cw_MQTTSUBSCRIBER));
	Init_cw_MQTTSUBSCRIBER(p, name, broker, topic);
	return p;
}
int cw_MQTTSUBSCRIBER_INIT_enter(struct cw_MQTTSUBSCRIBER *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_MQTTSUBSCRIBER_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	markPending(obj);
	return 1;
}
void cw_MQTTSUBSCRIBER_set_value (struct cw_MQTTSUBSCRIBER *m, const char *name, int *var, int value) {
	*var = value;
    publish_MQTT_property(0, &m->machine, name, value);
}
void Init_cw_MQTTSUBSCRIBER(struct cw_MQTTSUBSCRIBER *m, const char *name, MachineBase *broker, const char *topic) {
	initMachineBase(&m->machine, name);
    m->machine.class_name = "MQTTSUBSCRIBER";
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_broker = broker;
	if (broker) MachineDependencies_add(broker, cw_MQTTSUBSCRIBER_To_MachineBase(m));
	m->topic = topic;
	m->message = 0;
	m->VALUE = 0;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_MQTTSUBSCRIBER_check_state;
	m->machine.handle = (message_func)cw_MQTTSUBSCRIBER_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_MQTTSUBSCRIBER_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_MQTTSUBSCRIBER_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_MQTTSUBSCRIBER_describe;
	m->machine.set_value = (set_value_func)cw_MQTTSUBSCRIBER_set_value;
	m->vars = (struct cw_MQTTSUBSCRIBER_Vars *)malloc(sizeof(struct cw_MQTTSUBSCRIBER_Vars));
	m->backup = (struct cw_MQTTSUBSCRIBER_Vars_backup *)malloc(sizeof(struct cw_MQTTSUBSCRIBER_Vars_backup));
	init_Vars(m, m->vars);
	register_subscriber(m);
	MachineActions_add(cw_MQTTSUBSCRIBER_To_MachineBase(m), (enter_func)cw_MQTTSUBSCRIBER_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_MQTTSUBSCRIBER_getAddress(struct cw_MQTTSUBSCRIBER *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_MQTTSUBSCRIBER_To_MachineBase(struct cw_MQTTSUBSCRIBER *p) { return &p->machine; }

int cw_MQTTSUBSCRIBER_check_state(struct cw_MQTTSUBSCRIBER *m) {
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_MQTTSUBSCRIBER_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
void cw_MQTTSUBSCRIBER_describe(struct cw_MQTTSUBSCRIBER *m) {
	struct cw_MQTTBROKER *broker = (struct cw_MQTTBROKER *)m->_broker;
	char buf[100];
	snprintf(buf, 100, "%s: %s:%d/%s = %d Class: MQTTSUBSCRIBER", m->machine.name, broker->host, broker->port, m->topic, m->message);
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT(0, "/response", buf);
}
