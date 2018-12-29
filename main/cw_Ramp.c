
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_symbol_ids.h"
#include "cw_Ramp.h"
#define DEBUG_LOG 0
static const char* TAG = "Ramp";

#define state_cw_INIT 1
#define state_cw_bottom_forward 7
#define state_cw_bottom_reverse 8
#define state_cw_falling 9
#define state_cw_off 2
#define state_cw_on 3
#define state_cw_rising 10
#define state_cw_stopped 11
#define state_cw_top 12
struct cw_Ramp_Vars {
	struct cw_Ramp *m;
	unsigned int l_INIT;
	unsigned int l_bottom_forward;
	unsigned int l_bottom_reverse;
	unsigned int *l_clock;
	Value *l_direction;
	Value *l_end;
	unsigned int l_falling;
	unsigned int *l_forward;
	unsigned int l_off;
	unsigned int l_on;
	unsigned int *l_output;
	Value *l_output_VALUE;
	unsigned int l_rising;
	Value *l_start;
	Value *l_step;
	unsigned int l_stopped;
	unsigned int l_top;
};
static void init_Vars(struct cw_Ramp *m, struct cw_Ramp_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_bottom_forward = state_cw_bottom_forward;
	v->l_bottom_reverse = state_cw_bottom_reverse;
	v->l_clock = &m->_clock->state;
	v->l_direction = &m->direction;
	v->l_end = &m->end;
	v->l_falling = state_cw_falling;
	v->l_forward = &m->_forward->state;
	v->l_off = state_cw_off;
	v->l_on = state_cw_on;
	v->l_output = &m->_output->state;
	{
	MachineBase *mm = m;
	mm = mm->lookup_machine(mm, sym_output);
	v->l_output_VALUE = mm->lookup(mm, sym_VALUE);
	}
	v->l_rising = state_cw_rising;
	v->l_start = &m->start;
	v->l_step = &m->step;
	v->l_stopped = state_cw_stopped;
	v->l_top = state_cw_top;
}
Value *cw_Ramp_lookup(struct cw_Ramp *m, int symbol) {
	if (symbol == sym_direction) return &m->direction;
	if (symbol == sym_end) return &m->end;
	if (symbol == sym_start) return &m->start;
	if (symbol == sym_step) return &m->step;
	return 0;
}
MachineBase *cw_Ramp_lookup_machine(struct cw_Ramp *m, int symbol) {
	if (symbol == sym_clock) return m->_clock;
	if (symbol == sym_output) return m->_output;
	if (symbol == sym_forward) return m->_forward;
	return 0;
}
int cw_Ramp_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_Ramp_check_state(struct cw_Ramp *m);
struct cw_Ramp *create_cw_Ramp(const char *name, MachineBase *clock, MachineBase *output, MachineBase *forward) {
	struct cw_Ramp *p = (struct cw_Ramp *)malloc(sizeof(struct cw_Ramp));
	Init_cw_Ramp(p, name, clock, output, forward);
	return p;
}
int cw_Ramp_INIT_enter(struct cw_Ramp *m, ccrContParam) {
	struct cw_Ramp_Vars *v = m->vars;
// INIT 
	*v->l_output_VALUE = *v->l_start;
	*v->l_direction = 1;
	cw_send(m->_forward, m, cw_message_turnOn);
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_bottom_forward_enter(struct cw_Ramp *m, ccrContParam) {
	struct cw_Ramp_Vars *v = m->vars;
// bottom_forward 
	*v->l_output_VALUE = *v->l_start;
	*v->l_direction = 1;
	cw_send(m->_forward, m, cw_message_turnOff);
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_bottom_reverse_enter(struct cw_Ramp *m, ccrContParam) {
	struct cw_Ramp_Vars *v = m->vars;
// bottom_reverse 
	*v->l_output_VALUE = *v->l_start;
	*v->l_direction = 1;
	cw_send(m->_forward, m, cw_message_turnOn);
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_clock_on_enter(struct cw_Ramp *m, ccrContParam) {
	struct cw_Ramp_Vars *v = m->vars;
// clock.on_enter 
	*v->l_output_VALUE = (*v->l_output_VALUE + (*v->l_direction * *v->l_step));
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_top_enter(struct cw_Ramp *m, ccrContParam) {
	struct cw_Ramp_Vars *v = m->vars;
// top 
	*v->l_output_VALUE = *v->l_end;
	*v->l_direction = -1;
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_Ramp *m = (struct cw_Ramp *)obj;
	 if (source == m->_clock && state == 3)
		MachineActions_add(m, (enter_func)cw_Ramp_clock_on_enter);
	markPending(obj);
	return 1;
}
void Init_cw_Ramp(struct cw_Ramp *m, const char *name, MachineBase *clock, MachineBase *output, MachineBase *forward) {
	initMachineBase(&m->machine, name);
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_clock = clock;
	if (clock) MachineDependencies_add(clock, cw_Ramp_To_MachineBase(m));
	m->_output = output;
	if (output) MachineDependencies_add(output, cw_Ramp_To_MachineBase(m));
	m->_forward = forward;
	if (forward) MachineDependencies_add(forward, cw_Ramp_To_MachineBase(m));
	m->direction = 0;
	m->end = 30000;
	m->start = 1000;
	m->step = 800;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_Ramp_check_state;
	m->machine.handle = (message_func)cw_Ramp_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_Ramp_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_Ramp_lookup_machine; // lookup symbols within this machine
	m->vars = (struct cw_Ramp_Vars *)malloc(sizeof(struct cw_Ramp_Vars));
	init_Vars(m, m->vars);
	MachineActions_add(cw_Ramp_To_MachineBase(m), (enter_func)cw_Ramp_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_Ramp_getAddress(struct cw_Ramp *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_Ramp_To_MachineBase(struct cw_Ramp *p) { return &p->machine; }

int cw_Ramp_check_state(struct cw_Ramp *m) {
	struct cw_Ramp_Vars *v = m->vars;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	if (((*v->l_output_VALUE >= *v->l_end) && (*v->l_direction > 0))) {
		new_state = state_cw_top;
		new_state_enter = (enter_func)cw_Ramp_top_enter;
	}
	else
	if ((((*v->l_output_VALUE <= *v->l_start) && (*v->l_direction < 0)) && (*v->l_forward == v->l_off))) {
		new_state = state_cw_bottom_reverse;
		new_state_enter = (enter_func)cw_Ramp_bottom_reverse_enter;
	}
	else
	if ((((*v->l_output_VALUE <= *v->l_start) && (*v->l_direction < 0)) && (*v->l_forward == v->l_on))) {
		new_state = state_cw_bottom_forward;
		new_state_enter = (enter_func)cw_Ramp_bottom_forward_enter;
	}
	else
	if (((*v->l_output_VALUE < *v->l_end) && (*v->l_direction > 0))) {
		new_state = state_cw_rising;
	}
	else
	if (((*v->l_output_VALUE > *v->l_start) && (*v->l_direction < 0))) {
		new_state = state_cw_falling;
	}
	else
	{
		new_state = state_cw_stopped;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_Ramp_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
