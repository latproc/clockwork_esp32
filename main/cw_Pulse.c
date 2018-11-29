
#include "base_includes.h"
#include "cw_Pulse.h"
static const char* TAG = "Pulse";
#define DEBUG_LOG 0

#define Value int
struct cw_Pulse {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_out;
	Value delay; // 100
};
int cw_Pulse_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_Pulse_check_state(struct cw_Pulse *m);
struct cw_Pulse *create_cw_Pulse(const char *name, MachineBase *out) {
	struct cw_Pulse *p = (struct cw_Pulse *)malloc(sizeof(struct cw_Pulse));
	Init_cw_Pulse(p, name, out);
	return p;
}
int cw_Pulse_off_enter(struct cw_Pulse *m, ccrContParam) {// off 
	ESP_LOGI(TAG, "%lld %s", upTime(), "off");
	cw_send(m->_out, m, cw_message_turnOn);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_on_enter(struct cw_Pulse *m, ccrContParam) {// on 
	ESP_LOGI(TAG, "%lld %s", upTime(), "on");
	cw_send(m->_out, m, cw_message_turnOff);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_Pulse *m = (struct cw_Pulse *)obj;
	return 1;
}
uint64_t cw_Pulse_next_trigger_time(struct cw_Pulse *m) {
  uint64_t val = m->delay;
  uint64_t res = val;
  return res;
}
void Init_cw_Pulse(struct cw_Pulse *m, const char *name, MachineBase *out) {
	initMachineBase(&m->machine, name);
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_out = out;
	if (out) MachineDependencies_add(out, cw_Pulse_To_MachineBase(m));
	m->delay = 100;
	m->machine.state = state_cw_Pulse_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_Pulse_check_state;
	m->machine.handle = (message_func)cw_Pulse_handle_message; // handle message from other machines
	//MachineActions_add(cw_Pulse_To_MachineBase(m), (enter_func)cw_Pulse_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_Pulse_getAddress(struct cw_Pulse *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_Pulse_To_MachineBase(struct cw_Pulse *p) { return &p->machine; }
int cw_Pulse_check_state(struct cw_Pulse *m) {
	int new_state = 0; enter_func new_state_enter = 0;
	if (((m->machine.state == state_cw_Pulse_off) && (m->machine.TIMER >= m->delay))) {
		new_state = state_cw_Pulse_on;
		new_state_enter = (enter_func)cw_Pulse_on_enter;
	}
    else {
		new_state = state_cw_Pulse_off;
		new_state_enter = (enter_func)cw_Pulse_off_enter;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_Pulse_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		uint64_t delay = cw_Pulse_next_trigger_time(m);
		struct RTScheduler *scheduler = RTScheduler_get();
		while (!scheduler) {
			taskYIELD();
			scheduler = RTScheduler_get();
		}
		if (delay > m->machine.TIMER) RTScheduler_add(scheduler, ScheduleItem_create(delay - m->machine.TIMER, &m->machine));
		else
			if (m->machine.execute) markPending(&m->machine);
		RTScheduler_release();
		return 1;
	}
	return 0;
}
