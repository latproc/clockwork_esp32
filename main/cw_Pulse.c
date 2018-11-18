
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
	//LOG (empty);
	//SendMessageActionTemplate turnOff out;
    //ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off" );
	turnOn(m->_out);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_on_enter(struct cw_Pulse *m, ccrContParam) {// on 
	//LOG (empty);
	//SendMessageActionTemplate turnOn out;
    //ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on" );
	turnOff(m->_out);
	m->machine.execute = 0;
	return 1;
}
int cw_Pulse_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_Pulse *m = (struct cw_Pulse *)obj;
	return 1;
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
		changeMachineState(cw_Pulse_To_MachineBase(m), new_state, new_state_enter);
        struct RTScheduler *scheduler = RTScheduler_get();
        while (!scheduler) {
            taskYIELD();
            scheduler = RTScheduler_get();
        }
        if (m->delay > m->machine.TIMER) {
            RTScheduler_add(scheduler, ScheduleItem_create(m->delay - m->machine.TIMER, &m->machine));
            RTScheduler_release();
        }
    }
    if (m->machine.execute) markPending(&m->machine);
    return 1;
	
    //ESP_LOGI(TAG, "%ld [%d] %s",m->machine.TIMER, m->machine.id, (new_state == state_cw_Pulse_on) ? "on" : "off" );
	return 0;
}
