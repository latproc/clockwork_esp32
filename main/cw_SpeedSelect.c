
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_symbol_ids.h"
#include "cw_SpeedSelect.h"
#define DEBUG_LOG 0
static const char* TAG = "SpeedSelect";

#define state_cw_INIT 1
#define state_cw_fast 13
#define state_cw_slow 14
struct cw_SpeedSelect_Vars {
	struct cw_SpeedSelect *m;
	unsigned int l_INIT;
	unsigned int *l_button;
	unsigned int l_fast;
	unsigned int *l_pulser;
	Value *l_pulser_delay;
	unsigned int l_slow;
};
static void init_Vars(struct cw_SpeedSelect *m, struct cw_SpeedSelect_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_button = &m->_button->state;
	v->l_fast = state_cw_fast;
	v->l_pulser = &m->_pulser->state;
	{
	MachineBase *mm = m;
	mm = mm->lookup_machine(mm, sym_pulser);
	v->l_pulser_delay = mm->lookup(mm, sym_delay);
	}
	v->l_slow = state_cw_slow;
}
Value *cw_SpeedSelect_lookup(struct cw_SpeedSelect *m, int symbol) {
	return 0;
}
MachineBase *cw_SpeedSelect_lookup_machine(struct cw_SpeedSelect *m, int symbol) {
	if (symbol == sym_button) return m->_button;
	if (symbol == sym_pulser) return m->_pulser;
	return 0;
}
int cw_SpeedSelect_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_SpeedSelect_check_state(struct cw_SpeedSelect *m);
struct cw_SpeedSelect *create_cw_SpeedSelect(const char *name, MachineBase *button, MachineBase *pulser) {
	struct cw_SpeedSelect *p = (struct cw_SpeedSelect *)malloc(sizeof(struct cw_SpeedSelect));
	Init_cw_SpeedSelect(p, name, button, pulser);
	return p;
}
int cw_SpeedSelect_button_off_enter(struct cw_SpeedSelect *m, ccrContParam) {
	struct cw_SpeedSelect_Vars *v = m->vars;
// button.off_enter 
	cw_send(m->_pulser, m, cw_message_toggle_speed);
	m->machine.execute = 0;
	return 1;
}
int cw_SpeedSelect_INIT_enter(struct cw_SpeedSelect *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_SpeedSelect_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_SpeedSelect *m = (struct cw_SpeedSelect *)obj;
	 if (source == m->_button && state == 2)
		MachineActions_add(m, (enter_func)cw_SpeedSelect_button_off_enter);
	markPending(obj);
	return 1;
}
void Init_cw_SpeedSelect(struct cw_SpeedSelect *m, const char *name, MachineBase *button, MachineBase *pulser) {
	initMachineBase(&m->machine, name);
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_button = button;
	if (button) MachineDependencies_add(button, cw_SpeedSelect_To_MachineBase(m));
	m->_pulser = pulser;
	if (pulser) MachineDependencies_add(pulser, cw_SpeedSelect_To_MachineBase(m));
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_SpeedSelect_check_state;
	m->machine.handle = (message_func)cw_SpeedSelect_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_SpeedSelect_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_SpeedSelect_lookup_machine; // lookup symbols within this machine
	m->vars = (struct cw_SpeedSelect_Vars *)malloc(sizeof(struct cw_SpeedSelect_Vars));
	init_Vars(m, m->vars);
	MachineActions_add(cw_SpeedSelect_To_MachineBase(m), (enter_func)cw_SpeedSelect_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_SpeedSelect_getAddress(struct cw_SpeedSelect *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_SpeedSelect_To_MachineBase(struct cw_SpeedSelect *p) { return &p->machine; }

int cw_SpeedSelect_check_state(struct cw_SpeedSelect *m) {
	struct cw_SpeedSelect_Vars *v = m->vars;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	if ((*v->l_pulser_delay < 200)) {
		new_state = state_cw_fast;
	}
	else
	{
		new_state = state_cw_slow;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_SpeedSelect_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
