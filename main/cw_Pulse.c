
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_symbol_ids.h"
#include "cw_Pulse.h"
#define DEBUG_LOG 0
static const char* TAG = "Pulse";

#define state_cw_INIT 1
#define state_cw_off 2
#define state_cw_on 3
struct cw_Pulse_Vars {
	struct cw_Pulse *m;
	unsigned int l_INIT;
	unsigned int *l_SELF;
	unsigned long *l_TIMER;
	Value *l_delay;
	unsigned int l_off;
	unsigned int l_on;
	unsigned int *l_out;
};
static void init_Vars(struct cw_Pulse *m, struct cw_Pulse_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_SELF = &m->machine.state;
	v->l_TIMER = &m->machine.TIMER;
	v->l_delay = &m->delay;
	v->l_off = state_cw_off;
	v->l_on = state_cw_on;
	v->l_out = &m->_out->state;
}
Value *cw_Pulse_lookup(struct cw_Pulse *m, int symbol) {
	if (symbol == sym_delay) return &m->delay;
	return 0;
}
MachineBase *cw_Pulse_lookup_machine(struct cw_Pulse *m, int symbol) {
	if (symbol == sym_out) return m->_out;
	return 0;
}
int cw_Pulse_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_Pulse_check_state(struct cw_Pulse *m);
uint64_t cw_Pulse_next_trigger_time(struct cw_Pulse *m, struct cw_Pulse_Vars *v);
struct cw_Pulse *create_cw_Pulse(const char *name, MachineBase *out) {
	struct cw_Pulse *p = (struct cw_Pulse *)malloc(sizeof(struct cw_Pulse));
	Init_cw_Pulse(p, name, out);
	return p;
}
int cw_Pulse_off_enter(struct cw_Pulse *m, ccrContParam) {
	struct cw_Pulse_Vars *v = m->vars;
// off 
	ESP_LOGI(TAG, "%lld %s", upTime(), " off");
	cw_send(m->_out, m, cw_message_turnOff);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_on_enter(struct cw_Pulse *m, ccrContParam) {
	struct cw_Pulse_Vars *v = m->vars;
// on 
	ESP_LOGI(TAG, "%lld %s", upTime(), " on");
	cw_send(m->_out, m, cw_message_turnOn);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_toggle_speed(struct cw_Pulse *m, ccrContParam) {
	struct cw_Pulse_Vars *v = m->vars;
// toggle_speed 
	*v->l_delay = (1100 - *v->l_delay);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_INIT_enter(struct cw_Pulse *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_Pulse_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_Pulse *m = (struct cw_Pulse *)obj;
	if (state == cw_message_toggle_speed)
		MachineActions_add(m, (enter_func)cw_Pulse_toggle_speed);
	markPending(obj);
	return 1;
}
uint64_t cw_Pulse_next_trigger_time(struct cw_Pulse *m, struct cw_Pulse_Vars *v) {
	int64_t res = 1000000000;
	int64_t val = 0;
	//TODO: remove possible duplicates here
	val = *v->l_delay - *v->l_TIMER;
	if (val>0 && val < res) res = val;
	if (res == 1000000000) res = 0;
	return res;
}
void Init_cw_Pulse(struct cw_Pulse *m, const char *name, MachineBase *out) {
	initMachineBase(&m->machine, name);
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_out = out;
	if (out) MachineDependencies_add(out, cw_Pulse_To_MachineBase(m));
	m->delay = 100;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_Pulse_check_state;
	m->machine.handle = (message_func)cw_Pulse_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_Pulse_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_Pulse_lookup_machine; // lookup symbols within this machine
	m->vars = (struct cw_Pulse_Vars *)malloc(sizeof(struct cw_Pulse_Vars));
	init_Vars(m, m->vars);
	MachineActions_add(cw_Pulse_To_MachineBase(m), (enter_func)cw_Pulse_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_Pulse_getAddress(struct cw_Pulse *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_Pulse_To_MachineBase(struct cw_Pulse *p) { return &p->machine; }

int cw_Pulse_check_state(struct cw_Pulse *m) {
	struct cw_Pulse_Vars *v = m->vars;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	if (((*v->l_SELF == v->l_off) && (m->machine.TIMER >= *v->l_delay))) {
		new_state = state_cw_on;
		new_state_enter = (enter_func)cw_Pulse_on_enter;
	}
	else
	{
		new_state = state_cw_off;
		new_state_enter = (enter_func)cw_Pulse_off_enter;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_Pulse_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	uint64_t delay = cw_Pulse_next_trigger_time(m, v);
	if (delay > 0) {
		struct RTScheduler *scheduler = RTScheduler_get();
		while (!scheduler) {
			taskYIELD();
			scheduler = RTScheduler_get();
		}
		RTScheduler_add(scheduler, ScheduleItem_create(delay, &m->machine));
		RTScheduler_release();
	}
	return res;
}
