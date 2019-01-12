
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_symbol_ids.h"
#include "cw_DINPUT.h"
#define DEBUG_LOG 0
static const char* TAG = "DINPUT";

#define state_cw_INIT 1
#define state_cw_off 2
#define state_cw_on 3
struct cw_DINPUT_Vars {
	struct cw_DINPUT *m;
	unsigned int l_INIT;
	unsigned int *l_Input;
	unsigned long *l_Input_TIMER;
	unsigned int *l_SELF;
	unsigned int l_off;
	unsigned int l_on;
	Value *l_stable;
};
struct cw_DINPUT_Vars_backup {
	struct cw_DINPUT  m;
	unsigned int l_INIT;
	unsigned int  l_Input;
	unsigned long  l_Input_TIMER;
	unsigned int  l_SELF;
	unsigned int l_off;
	unsigned int l_on;
	Value  l_stable;
};
static void init_Vars(struct cw_DINPUT *m, struct cw_DINPUT_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_Input = &m->_Input->state;
	{
	MachineBase *mm = &m->machine;
	mm = mm->lookup_machine(mm, sym_Input);
	v->l_Input_TIMER = &mm->TIMER;
	}
	v->l_SELF = &m->machine.state;
	v->l_off = state_cw_off;
	v->l_on = state_cw_on;
	v->l_stable = &m->stable;
}
static void backup_Vars(struct cw_DINPUT *m) {
	struct cw_DINPUT_Vars *v = m->vars;
	struct cw_DINPUT_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_Input = *v->l_Input;
	b->l_Input_TIMER = *v->l_Input_TIMER;
	b->l_SELF = *v->l_SELF;
	b->l_off = v->l_off;
	b->l_on = v->l_on;
	b->l_stable = *v->l_stable;
}
Value *cw_DINPUT_lookup(struct cw_DINPUT *m, int symbol) {
	if (symbol == sym_stable) return &m->stable;
	return 0;
}
MachineBase *cw_DINPUT_lookup_machine(struct cw_DINPUT *m, int symbol) {
	if (symbol == sym_Input) return m->_Input;
	return 0;
}
void cw_DINPUT_describe(struct cw_DINPUT *m);
int cw_DINPUT_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_DINPUT_check_state(struct cw_DINPUT *m);
uint64_t cw_DINPUT_next_trigger_time(struct cw_DINPUT *m, struct cw_DINPUT_Vars_backup *v);
struct cw_DINPUT *create_cw_DINPUT(const char *name, MachineBase *Input) {
	struct cw_DINPUT *p = (struct cw_DINPUT *)malloc(sizeof(struct cw_DINPUT));
	Init_cw_DINPUT(p, name, Input);
	return p;
}
int cw_DINPUT_INIT_enter(struct cw_DINPUT *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_DINPUT_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_DINPUT *m = (struct cw_DINPUT *)obj;
	markPending(obj);
	return 1;
}
uint64_t cw_DINPUT_next_trigger_time(struct cw_DINPUT *m, struct cw_DINPUT_Vars_backup *v) {
	int64_t res = 1000000000;
	int64_t val = 0;
	//TODO: remove possible duplicates here
	val = v->l_stable - v->l_Input_TIMER;
	if (val>0 && val < res) res = val;
	val = v->l_stable - v->l_Input_TIMER;
	if (val>0 && val < res) res = val;
	val = v->l_stable - v->l_Input_TIMER;
	if (val>0 && val < res) res = val;
	if (res == 1000000000) res = 0;
	return res;
}
void Init_cw_DINPUT(struct cw_DINPUT *m, const char *name, MachineBase *Input) {
	initMachineBase(&m->machine, name);
	m->machine.class_name = "DINPUT";
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_Input = Input;
	if (Input) MachineDependencies_add(Input, cw_DINPUT_To_MachineBase(m));
	m->stable = 50;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_DINPUT_check_state;
	m->machine.handle = (message_func)cw_DINPUT_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_DINPUT_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_DINPUT_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_DINPUT_describe;
	m->vars = (struct cw_DINPUT_Vars *)malloc(sizeof(struct cw_DINPUT_Vars));
	m->backup = (struct cw_DINPUT_Vars_backup *)malloc(sizeof(struct cw_DINPUT_Vars_backup));
	init_Vars(m, m->vars);
	MachineActions_add(cw_DINPUT_To_MachineBase(m), (enter_func)cw_DINPUT_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_DINPUT_getAddress(struct cw_DINPUT *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_DINPUT_To_MachineBase(struct cw_DINPUT *p) { return &p->machine; }

int cw_DINPUT_check_state(struct cw_DINPUT *m) {
	struct cw_DINPUT_Vars_backup *v = m->backup;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if ((((v->l_SELF == v->l_on) && (v->l_Input == v->l_off)) && (v->l_Input_TIMER >= v->l_stable))) /* off */ {
		new_state = state_cw_off;
	}
	else
	if (((v->l_SELF == v->l_on) || ((v->l_Input == v->l_on) && (v->l_Input_TIMER >= v->l_stable)))) /* on */ {
		new_state = state_cw_on;
	}
	else
	if (((v->l_SELF == v->l_off) || ((v->l_Input == v->l_off) && (v->l_Input_TIMER >= v->l_stable)))) /* off */ {
		new_state = state_cw_off;
	}
	else
	{
		new_state = state_cw_off;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_DINPUT_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	uint64_t delay = cw_DINPUT_next_trigger_time(m, v);
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
void cw_DINPUT_describe(struct cw_DINPUT *m) {
	struct cw_DINPUT_Vars_backup *v = m->backup;
{
	char buf[100];
	snprintf(buf, 100, "%s: %s  Class: DINPUT", m->machine.name, name_from_id(m->machine.state));
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT(0,"/response", buf);
}
	char buf[200];
	snprintf(buf, 200, "off [%d]: (((SELF (%ld) == on (%ld)) && (Input (%ld) == off (%ld))) && (Input_TIMER (%ld) >= stable (%ld)))",state_cw_off,(long)v->l_SELF,(long)v->l_on,(long)v->l_Input,(long)v->l_off,(long)v->l_Input_TIMER,(long)v->l_stable);
	sendMQTT(0, "/response", buf);
	snprintf(buf, 200, "on [%d]: ((SELF (%ld) == on (%ld)) || ((Input (%ld) == on (%ld)) && (Input_TIMER (%ld) >= stable (%ld))))",state_cw_on,(long)v->l_SELF,(long)v->l_on,(long)v->l_Input,(long)v->l_on,(long)v->l_Input_TIMER,(long)v->l_stable);
	sendMQTT(0, "/response", buf);
	snprintf(buf, 200, "off [%d]: ((SELF (%ld) == off (%ld)) || ((Input (%ld) == off (%ld)) && (Input_TIMER (%ld) >= stable (%ld))))",state_cw_off,(long)v->l_SELF,(long)v->l_off,(long)v->l_Input,(long)v->l_off,(long)v->l_Input_TIMER,(long)v->l_stable);
	sendMQTT(0, "/response", buf);
}
