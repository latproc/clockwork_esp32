
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_Ramp.h"
static const char* TAG = "Ramp";
#define DEBUG_LOG 0
int cw_Ramp_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_Ramp_check_state(struct cw_Ramp *m);
struct cw_Ramp *create_cw_Ramp(const char *name, MachineBase *clock, MachineBase *output) {
	struct cw_Ramp *p = (struct cw_Ramp *)malloc(sizeof(struct cw_Ramp));
	Init_cw_Ramp(p, name, clock, output);
	return p;
}
int cw_Ramp_INIT_enter(struct cw_Ramp *m, ccrContParam) {// INIT 
	m->VALUE = m->start;
	m->direction = 1;
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_bottom_enter(struct cw_Ramp *m, ccrContParam) {// bottom 
	m->VALUE = m->start;
	m->direction = 1;
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_clock_on_enter(struct cw_Ramp *m, ccrContParam) {// clock.on_enter 
	m->VALUE = (m->VALUE + (m->direction * m->step));
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_top_enter(struct cw_Ramp *m, ccrContParam) {// top 
	m->VALUE = m->end;
	m->direction = -1;
	m->machine.execute = 0;
	return 1;
}
int cw_Ramp_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_Ramp *m = (struct cw_Ramp *)obj;
	 if (source == m->_clock && state == 3)
		MachineActions_add(m, (enter_func)cw_Ramp_clock_on_enter);
	return 1;
}
void Init_cw_Ramp(struct cw_Ramp *m, const char *name, MachineBase *clock, MachineBase *output) {
	initMachineBase(&m->machine, name);
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_clock = clock;
	if (clock) MachineDependencies_add(clock, cw_Ramp_To_MachineBase(m));
	m->_output = output;
	if (output) MachineDependencies_add(output, cw_Ramp_To_MachineBase(m));
	m->VALUE = 0;
	m->direction = 0;
	m->end = 30000;
	m->start = 1000;
	m->step = 800;
	m->machine.state = state_cw_Ramp_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_Ramp_check_state;
	m->machine.handle = (message_func)cw_Ramp_handle_message; // handle message from other machines
	MachineActions_add(cw_Ramp_To_MachineBase(m), (enter_func)cw_Ramp_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_Ramp_getAddress(struct cw_Ramp *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_Ramp_To_MachineBase(struct cw_Ramp *p) { return &p->machine; }

int cw_Ramp_check_state(struct cw_Ramp *m) {
	int new_state = 0; enter_func new_state_enter = 0;
	if (((m->VALUE >= m->end) && (m->direction > 0))) {
		new_state = state_cw_Ramp_top;
		new_state_enter = (enter_func)cw_Ramp_top_enter;
	}
	else
	if (((m->VALUE <= m->start) && (m->direction < 0))) {
		new_state = state_cw_Ramp_bottom;
		new_state_enter = (enter_func)cw_Ramp_bottom_enter;
	}
	else
	if (((m->VALUE < m->end) && (m->direction > 0))) {
		new_state = state_cw_Ramp_rising;
	}
	else
	if (((m->VALUE > m->start) && (m->direction < 0))) {
		new_state = state_cw_Ramp_falling;
	}
	else
	{
		new_state = state_cw_Ramp_stopped;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_Ramp_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		return 1;
	}
	return 0;
}
