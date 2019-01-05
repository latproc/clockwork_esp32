
#include "base_includes.h"
#include "cw_SYSTEMSETTINGS.h"
#define DEBUG_LOG 0
//static const char* TAG = "SYSTEMSETTINGS";

#define state_cw_INIT 1
#define state_cw_ready -1
struct cw_SYSTEMSETTINGS_Vars {
	struct cw_SYSTEMSETTINGS *m;
	Value *l_CYCLE_DELAY;
	const char *l_HOST;
	const char *l_INFO;
	unsigned int l_INIT;
	Value *l_POLLING_DELAY;
	Value *l_VERSION;
	unsigned int l_ready;
};
struct cw_SYSTEMSETTINGS_Vars_backup {
	struct cw_SYSTEMSETTINGS  m;
	Value  l_CYCLE_DELAY;
	const char *l_HOST;
	const char *l_INFO;
	unsigned int l_INIT;
	Value  l_POLLING_DELAY;
	Value  l_VERSION;
	unsigned int l_ready;
};
static void init_Vars(struct cw_SYSTEMSETTINGS *m, struct cw_SYSTEMSETTINGS_Vars *v) {
	v->m = m;
	v->l_CYCLE_DELAY = &m->CYCLE_DELAY;
	v->l_HOST = m->HOST;
	v->l_INFO = m->INFO;
	v->l_INIT = state_cw_INIT;
	v->l_POLLING_DELAY = &m->POLLING_DELAY;
	v->l_VERSION = &m->VERSION;
	v->l_ready = state_cw_ready;
}
static void backup_Vars(struct cw_SYSTEMSETTINGS *m) {
	struct cw_SYSTEMSETTINGS_Vars *v = m->vars;
	struct cw_SYSTEMSETTINGS_Vars_backup *b = m->backup;
	b->l_CYCLE_DELAY = *v->l_CYCLE_DELAY;
	b->l_HOST = v->l_HOST;
	b->l_INFO = v->l_INFO;
	b->l_INIT = v->l_INIT;
	b->l_POLLING_DELAY = *v->l_POLLING_DELAY;
	b->l_VERSION = *v->l_VERSION;
	b->l_ready = v->l_ready;
}
Value *cw_SYSTEMSETTINGS_lookup(struct cw_SYSTEMSETTINGS *m, int symbol) {
	return 0;
}
MachineBase *cw_SYSTEMSETTINGS_lookup_machine(struct cw_SYSTEMSETTINGS *m, int symbol) {
	return 0;
}
void cw_SYSTEMSETTINGS_describe(struct cw_SYSTEMSETTINGS *m);
int cw_SYSTEMSETTINGS_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_SYSTEMSETTINGS_check_state(struct cw_SYSTEMSETTINGS *m);
struct cw_SYSTEMSETTINGS *create_cw_SYSTEMSETTINGS(const char *name) {
	struct cw_SYSTEMSETTINGS *p = (struct cw_SYSTEMSETTINGS *)malloc(sizeof(struct cw_SYSTEMSETTINGS));
	Init_cw_SYSTEMSETTINGS(p, name);
	return p;
}
int cw_SYSTEMSETTINGS_INIT_enter(struct cw_SYSTEMSETTINGS *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_SYSTEMSETTINGS_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	markPending(obj);
	return 1;
}
void Init_cw_SYSTEMSETTINGS(struct cw_SYSTEMSETTINGS *m, const char *name) {
	initMachineBase(&m->machine, name);
	m->CYCLE_DELAY = 2000;
	m->HOST = "";
	m->INFO = esp_get_idf_version();
	m->POLLING_DELAY = 2000;
	m->VERSION = 0.9;
	m->machine.state = state_cw_ready;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_SYSTEMSETTINGS_check_state;
	m->machine.handle = (message_func)cw_SYSTEMSETTINGS_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_SYSTEMSETTINGS_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_SYSTEMSETTINGS_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_SYSTEMSETTINGS_describe;
	m->vars = (struct cw_SYSTEMSETTINGS_Vars *)malloc(sizeof(struct cw_SYSTEMSETTINGS_Vars));
	m->backup = (struct cw_SYSTEMSETTINGS_Vars_backup *)malloc(sizeof(struct cw_SYSTEMSETTINGS_Vars_backup));
	init_Vars(m, m->vars);
	markPending(&m->machine);
}

int cw_SYSTEMSETTINGS_check_state(struct cw_SYSTEMSETTINGS *m) {
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if (new_state && new_state != m->machine.state) {
		changeMachineState(&m->machine, new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
void cw_SYSTEMSETTINGS_describe(struct cw_SYSTEMSETTINGS *m) {
	char buf[100];
	snprintf(buf, 100, "%s:  esp-idf version: %s", m->machine.name, m->INFO);
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld, Free memory: %u", m->machine.TIMER, esp_get_free_heap_size());
	sendMQTT(0, "/response", buf);
}
