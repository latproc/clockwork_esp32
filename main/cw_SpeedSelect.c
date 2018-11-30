
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_SpeedSelect.h"
#include "cw_Pulse.h"
static const char* TAG = "SpeedSelect";
#define DEBUG_LOG 0
int cw_SpeedSelect_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_SpeedSelect_check_state(struct cw_SpeedSelect *m);
struct cw_SpeedSelect *create_cw_SpeedSelect(const char *name, MachineBase *button, MachineBase *pulser) {
	struct cw_SpeedSelect *p = (struct cw_SpeedSelect *)malloc(sizeof(struct cw_SpeedSelect));
	Init_cw_SpeedSelect(p, name, button, pulser);
	return p;
}
int cw_SpeedSelect_button_off_enter(struct cw_SpeedSelect *m, ccrContParam) {// button.off_enter 
	cw_send(m->_pulser, m, cw_message_toggle_speed);
	m->machine.execute = 0;
	return 1;
}
int cw_SpeedSelect_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_SpeedSelect *m = (struct cw_SpeedSelect *)obj;
	 if (source == m->_button && state == 2)
		MachineActions_add(m, (enter_func)cw_SpeedSelect_button_off_enter);
	else
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
	m->machine.state = state_cw_SpeedSelect_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_SpeedSelect_check_state;
	m->machine.handle = (message_func)cw_SpeedSelect_handle_message; // handle message from other machines
	//MachineActions_add(cw_SpeedSelect_To_MachineBase(m), (enter_func)cw_SpeedSelect_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_SpeedSelect_getAddress(struct cw_SpeedSelect *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_SpeedSelect_To_MachineBase(struct cw_SpeedSelect *p) { return &p->machine; }

int cw_SpeedSelect_check_state(struct cw_SpeedSelect *m) {
	int new_state = 0; enter_func new_state_enter = 0;
 	struct cw_Pulse *pulser = (struct cw_Pulse *)m->_pulser;
 	if ((pulser->delay < 200)) {
		new_state = state_cw_SpeedSelect_fast;
	}
	else
	{
		new_state = state_cw_SpeedSelect_slow;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_SpeedSelect_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		return 1;
	}
	return 0;
}
