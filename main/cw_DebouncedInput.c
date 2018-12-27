
#include "base_includes.h"
#include "cw_message_ids.h"
#include "cw_DebouncedInput.h"
#define DEBUG_LOG 0
#if DEBUG_LOG
static const char* TAG = "DebouncedInput";
#endif

int cw_DebouncedInput_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_DebouncedInput_check_state(struct cw_DebouncedInput *m);
uint64_t cw_DebouncedInput_next_trigger_time(struct cw_DebouncedInput *m);
struct cw_DebouncedInput *create_cw_DebouncedInput(const char *name, MachineBase *in) {
	struct cw_DebouncedInput *p = (struct cw_DebouncedInput *)malloc(sizeof(struct cw_DebouncedInput));
	Init_cw_DebouncedInput(p, name, in);
	return p;
}
int cw_DebouncedInput_off_enter(struct cw_DebouncedInput *m, ccrContParam) {// off 
#if DEBUG_LOG
	ESP_LOGI(TAG, "%lld %s", upTime(), "Debounce input off");
#endif
	m->machine.execute = 0;
	return 1;
}
int cw_DebouncedInput_on_enter(struct cw_DebouncedInput *m, ccrContParam) {// on 
#if DEBUG_LOG
	ESP_LOGI(TAG, "%lld %s", upTime(), "Debounced input on");
#endif
	m->machine.execute = 0;
	return 1;
}
int cw_DebouncedInput_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	struct cw_DebouncedInput *m = (struct cw_DebouncedInput *)obj;
	markPending(obj);
	return 1;
}
uint64_t cw_DebouncedInput_next_trigger_time(struct cw_DebouncedInput *m) {
    int64_t res = 1000000000;
    int64_t val = m->debounce_time - m->machine.TIMER;
	if (val > 0 && val < res) res = val;
	val = m->off_time - m->_in->TIMER;
	if (val > 0 && val < res) res = val;
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
	m->machine.state = state_cw_DebouncedInput_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_DebouncedInput_check_state;
	m->machine.handle = (message_func)cw_DebouncedInput_handle_message; // handle message from other machines
	//MachineActions_add(cw_DebouncedInput_To_MachineBase(m), (enter_func)cw_DebouncedInput_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_DebouncedInput_getAddress(struct cw_DebouncedInput *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_DebouncedInput_To_MachineBase(struct cw_DebouncedInput *p) { return &p->machine; }
int cw_DebouncedInput_check_state(struct cw_DebouncedInput *m) {
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	if ((((m->_in->state == state_cw_DebouncedInput_off) && (m->_in->TIMER >= m->debounce_time)) || ((m->machine.state == state_cw_DebouncedInput_off) && (m->machine.TIMER < m->off_time)))) {
		new_state = state_cw_DebouncedInput_off;
		new_state_enter = (enter_func)cw_DebouncedInput_off_enter;
	}
	else
	{
		new_state = state_cw_DebouncedInput_on;
		new_state_enter = (enter_func)cw_DebouncedInput_on_enter;
	}
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_DebouncedInput_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	uint64_t delay = cw_DebouncedInput_next_trigger_time(m);
	if (delay > 0) {
		struct RTScheduler *scheduler = RTScheduler_get();
		while (!scheduler) {
			taskYIELD();
			scheduler = RTScheduler_get();
		}
		RTScheduler_add(scheduler, ScheduleItem_create(delay, &m->machine));
		RTScheduler_release();
	}
	else markPending(&m->machine);
	return res;
}
