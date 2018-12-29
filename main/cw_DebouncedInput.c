
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_symbol_ids.h"
#include "cw_DebouncedInput.h"
#define DEBUG_LOG 0
static const char* TAG = "DebouncedInput";

#define state_cw_INIT 1
#define state_cw_off 2
#define state_cw_on 3
struct cw_DebouncedInput_Vars {
	struct cw_DebouncedInput *m;
	unsigned int l_INIT;
	unsigned int *l_SELF;
	unsigned long *l_TIMER;
	Value *l_debounce_time;
	unsigned int *l_in;
	unsigned long *l_in_TIMER;
	unsigned int l_off;
	Value *l_off_time;
	unsigned int l_on;
};
static void init_Vars(struct cw_DebouncedInput *m, struct cw_DebouncedInput_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_SELF = &m->machine.state;
	v->l_TIMER = &m->machine.TIMER;
	v->l_debounce_time = &m->debounce_time;
	v->l_in = &m->_in->state;
	{
	MachineBase *mm = m;
	mm = mm->lookup_machine(mm, sym_in);
	v->l_in_TIMER = &mm->TIMER;
	}
	v->l_off = state_cw_off;
	v->l_off_time = &m->off_time;
	v->l_on = state_cw_on;
}
Value *cw_DebouncedInput_lookup(struct cw_DebouncedInput *m, int symbol) {
	if (symbol == sym_debounce_time) return &m->debounce_time;
	if (symbol == sym_off_time) return &m->off_time;
	return 0;
}
MachineBase *cw_DebouncedInput_lookup_machine(struct cw_DebouncedInput *m, int symbol) {
	if (symbol == sym_in) return m->_in;
	return 0;
}
int cw_DebouncedInput_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_DebouncedInput_check_state(struct cw_DebouncedInput *m);
uint64_t cw_DebouncedInput_next_trigger_time(struct cw_DebouncedInput *m, struct cw_DebouncedInput_Vars *v);
struct cw_DebouncedInput *create_cw_DebouncedInput(const char *name, MachineBase *in) {
	struct cw_DebouncedInput *p = (struct cw_DebouncedInput *)malloc(sizeof(struct cw_DebouncedInput));
	Init_cw_DebouncedInput(p, name, in);
	return p;
}
int cw_DebouncedInput_off_enter(struct cw_DebouncedInput *m, ccrContParam) {
	struct cw_DebouncedInput_Vars *v = m->vars;
// off 
	ESP_LOGI(TAG, "%lld %s", upTime(), "Debounce input off");
	m->machine.execute = 0;
	return 1;
}
int cw_DebouncedInput_on_enter(struct cw_DebouncedInput *m, ccrContParam) {
	struct cw_DebouncedInput_Vars *v = m->vars;
// on 
	ESP_LOGI(TAG, "%lld %s", upTime(), "Debounced input on");
	m->machine.execute = 0;
	return 1;
}
int cw_DebouncedInput_INIT_enter(struct cw_DebouncedInput *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_DebouncedInput_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_DebouncedInput *m = (struct cw_DebouncedInput *)obj;
	markPending(obj);
	return 1;
}
uint64_t cw_DebouncedInput_next_trigger_time(struct cw_DebouncedInput *m, struct cw_DebouncedInput_Vars *v) {
	int64_t res = 1000000000;
	int64_t val = 0;
	//TODO: remove possible duplicates here
	val = *v->l_debounce_time - *v->l_in_TIMER;
	if (val>0 && val < res) res = val;
	val = *v->l_off_time - *v->l_TIMER;
	if (val>0 && val < res) res = val;
	if (res == 1000000000) res = 0;
	return res;
}
void Init_cw_DebouncedInput(struct cw_DebouncedInput *m, const char *name, MachineBase *in) {
	initMachineBase(&m->machine, name);
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_in = in;
	if (in) MachineDependencies_add(in, cw_DebouncedInput_To_MachineBase(m));
	m->debounce_time = 100;
	m->off_time = 50;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_DebouncedInput_check_state;
	m->machine.handle = (message_func)cw_DebouncedInput_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_DebouncedInput_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_func)cw_DebouncedInput_lookup_machine; // lookup symbols within this machine
	m->vars = (struct cw_DebouncedInput_Vars *)malloc(sizeof(struct cw_DebouncedInput_Vars));
	init_Vars(m, m->vars);
	MachineActions_add(cw_DebouncedInput_To_MachineBase(m), (enter_func)cw_DebouncedInput_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_DebouncedInput_getAddress(struct cw_DebouncedInput *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_DebouncedInput_To_MachineBase(struct cw_DebouncedInput *p) { return &p->machine; }

int cw_DebouncedInput_check_state(struct cw_DebouncedInput *m) {
	struct cw_DebouncedInput_Vars *v = m->vars;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	if ((((*v->l_in == v->l_off) && (*v->l_in_TIMER >= *v->l_debounce_time)) || ((*v->l_SELF == v->l_off) && (m->machine.TIMER < *v->l_off_time)))) {
		new_state = state_cw_off;
		new_state_enter = (enter_func)cw_DebouncedInput_off_enter;
	}
	else
	{
		new_state = state_cw_on;
		new_state_enter = (enter_func)cw_DebouncedInput_on_enter;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_DebouncedInput_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	uint64_t delay = cw_DebouncedInput_next_trigger_time(m, v);
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