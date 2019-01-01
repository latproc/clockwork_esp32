
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
struct cw_Pulse_Vars_backup {
	struct cw_Pulse  m;
	unsigned int l_INIT;
	unsigned int  l_SELF;
	unsigned long  l_TIMER;
	Value  l_delay;
	unsigned int l_off;
	unsigned int l_on;
	unsigned int  l_out;
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
static void backup_Vars(struct cw_Pulse *m) {
	struct cw_Pulse_Vars *v = m->vars;
	struct cw_Pulse_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_SELF = *v->l_SELF;
	b->l_TIMER = *v->l_TIMER;
	b->l_delay = *v->l_delay;
	b->l_off = v->l_off;
	b->l_on = v->l_on;
	b->l_out = *v->l_out;
}
Value *cw_Pulse_lookup(struct cw_Pulse *m, int symbol) {
	if (symbol == sym_delay) return &m->delay;
	return 0;
}
MachineBase *cw_Pulse_lookup_machine(struct cw_Pulse *m, int symbol) {
	if (symbol == sym_out) return m->_out;
	return 0;
}
void cw_Pulse_describe(struct cw_Pulse *m);
int cw_Pulse_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_Pulse_check_state(struct cw_Pulse *m);
uint64_t cw_Pulse_next_trigger_time(struct cw_Pulse *m, struct cw_Pulse_Vars *v);
struct cw_Pulse *create_cw_Pulse(const char *name, MachineBase *out) {
	struct cw_Pulse *p = (struct cw_Pulse *)malloc(sizeof(struct cw_Pulse));
	Init_cw_Pulse(p, name, out);
	return p;
}
int cw_Pulse_off_enter(struct cw_Pulse *m, ccrContParam) {
	struct cw_Pulse_Vars *v;
	v = m->vars;
	ESP_LOGI(TAG, "%lld %s", upTime(), " off");
	cw_send(m->_out, &m->machine, cw_message_turnOff);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_on_enter(struct cw_Pulse *m, ccrContParam) {
	struct cw_Pulse_Vars *v;
	v = m->vars;
	ESP_LOGI(TAG, "%lld %s", upTime(), " on");
	cw_send(m->_out, &m->machine, cw_message_turnOn);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_toggle_speed(struct cw_Pulse *m, ccrContParam) {
	ccrBeginContext;
	struct cw_Pulse_Vars *v;
	unsigned long wait_start;
	ccrEndContext(ctx);
	ccrBegin(ctx);
	ctx->v = m->vars;
	ctx->wait_start = m->machine.TIMER;
	while (m->machine.TIMER - ctx->wait_start < 15) {
	  struct RTScheduler *scheduler = RTScheduler_get();
	  while (!scheduler) {
	    taskYIELD();
	    scheduler = RTScheduler_get();
	  }
	  goto_sleep(&m->machine);
	  RTScheduler_add(scheduler, ScheduleItem_create(15 - (m->machine.TIMER - ctx->wait_start), &m->machine));
	  RTScheduler_release();
	  ccrReturn(0);
	}
	*ctx->v->l_delay = (1100 - *ctx->v->l_delay);
	m->machine.execute = 0;
	ccrFinish(1);
}
int cw_Pulse_INIT_enter(struct cw_Pulse *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_Pulse_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_Pulse *m = (struct cw_Pulse *)obj;
	if (state == cw_message_toggle_speed)
		MachineActions_add(obj, (enter_func)cw_Pulse_toggle_speed);
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
	m->machine.describe = (describe_func)cw_Pulse_describe;
	m->vars = (struct cw_Pulse_Vars *)malloc(sizeof(struct cw_Pulse_Vars));
	m->backup = (struct cw_Pulse_Vars_backup *)malloc(sizeof(struct cw_Pulse_Vars_backup));
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
	backup_Vars(m);
	if (((*v->l_SELF == v->l_off) && (m->machine.TIMER >= *v->l_delay))) /* on */ {
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
void cw_Pulse_describe(struct cw_Pulse *m) {
	struct cw_Pulse_Vars_backup *v = m->backup;
{
	char buf[100];
	snprintf(buf, 100, "%s: %s  Class: Pulse", m->machine.name, name_from_id(m->machine.state));
	sendMQTT("/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT("/response", buf);
}
	char buf[200];
	snprintf(buf, 200, "on [%d]: ((SELF (%ld) == off (%ld)) && (TIMER (%ld) >= delay (%ld)))",state_cw_on,(long)v->l_SELF,(long)v->l_off,(long)v->l_TIMER,(long)v->l_delay);
	sendMQTT("/response", buf);
}
