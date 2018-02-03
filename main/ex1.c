#include <malloc.h>
#include "runtime.h"
#include "rtscheduler.h"
#include "ex1.h"
#include <esp_log.h>
#include <pointinput.h>
#include <pointoutput.h>
#include "coroutine.h"
#include "rtio.h"

static const char* TAG = "Ex1";

#define state_Pulse_on 1
#define state_Pulse_off 2

/* Pulse MACHINE {  */
struct Pulse {
	MachineBase machine;
    ccrContext enter_on;
    struct PointOutput *out;
};

void Init_Pulse(struct Pulse *m, const char *name, struct PointOutput *out);
int pulse_enter_on(struct Pulse *m, ccrContParam);
int pulse_enter_off(struct Pulse *m, ccrContParam);

MachineBase *Pulse_To_MachineBase(struct Pulse *p) { return &p->machine; }

struct Pulse *create_Pulse(const char *name, struct PointOutput *out) {
    struct Pulse  *p = (struct Pulse *)malloc(sizeof(struct Pulse));
	Init_Pulse(p, name, out);
	return p;
}

int Pulse_check_state(struct Pulse *m) {
	int found = 0;
    long delay_time = 100;
    /* on WHEN SELF IS off AND TIMER>=1000; */
	if (m->machine.state == state_Pulse_off && m->machine.TIMER >= delay_time) found = state_Pulse_on;
    /* off DEFAULT; */
	if (found == 0) { found = state_Pulse_off; }
	if (found && found != m->machine.state) {
		//ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, (found == state_Pulse_on) ? "on" : "off" );
        changeMachineState(&m->machine, found, (found == state_Pulse_on) ? pulse_enter_on : pulse_enter_off);
        if (found == state_Pulse_off) {
            struct RTScheduler *scheduler = RTScheduler_get();
            while (!scheduler) {
                taskYIELD();
                scheduler = RTScheduler_get();
            }
    		RTScheduler_add(scheduler, ScheduleItem_create(delay_time, &m->machine));
            RTScheduler_release();
        }
		if (m->machine.execute) markPending(&m->machine);
        return 1;
	}
	return 0;
}

void Init_Pulse(struct Pulse *m, const char *name, struct PointOutput *out) {
	initMachineBase(&m->machine, name);
	m->machine.flags &= MASK_PASSIVE; /* not a passive machine */
	m->machine.check_state = ( int(*)(MachineBase*) )Pulse_check_state;
    m->out = out;
	markPending(&m->machine);
    assert(xSemaphoreGiveRecursive(runtime_mutex) == pdFAIL);
}

int pulse_enter_on(struct Pulse *m, ccrContParam) {
/*
    ccrBeginContext;
    int i;
    ccrEndContext(enter_on);
    ccrBegin(enter_on);
    for (enter_on->i = 4; enter_on->i > 0; --enter_on->i) {
    	ESP_LOGI(TAG, "%lld [%d] %s %d",upTime(), m->machine.id, "on", enter_on->i);
        ccrReturn(0);
    }
*/
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
    turnOff(m->out);
    m->machine.execute = 0;
    return 1;
/*
    ccrFinish(1);
*/
}

int pulse_enter_off(struct Pulse *m, ccrContParam) {
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
    turnOn(m->out);
    m->machine.execute = 0;
    return 1;
}


