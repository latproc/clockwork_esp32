//
//  PointInput.cpp
//  Project: cwrt
//
//	All rights reserved. Use of this source code is governed by the
//	3-clause BSD License in LICENSE.txt.

#include "base_includes.h"

#define DEBUG_LOG 0
#if DEBUG_LOG
static const char* TAG = "PointInput";
#endif

struct PointInput {
	MachineBase machine;
    int gpio_pin;
    struct IOAddress addr;
};

struct PointInput *create_cw_PointInput(const char *name, int gpio) {
    struct PointInput *p = (struct PointInput *)malloc(sizeof(struct PointInput));
	Init_PointInput(p, name, gpio);
	return p;
}

int point_input_enter_on(struct PointInput *m, ccrContParam);
int point_input_enter_off(struct PointInput *m, ccrContParam);

int PointInput_check_state(struct PointInput *m) {
    if (m->addr.status == IO_DONE) { // state change
        m->addr.status = IO_STABLE;
        uint8_t val = rt_get_io_bit(&m->addr);
#if DEBUG_LOG
        ESP_LOGI(TAG,"io value is now %d (%d)", m->addr.value.u8, val);
#endif
        if (val)
            changeMachineState(cw_PointInput_To_MachineBase(m), state_PointInput_on, (enter_func)point_input_enter_on);
        else
            changeMachineState(cw_PointInput_To_MachineBase(m), state_PointInput_off, (enter_func)point_input_enter_off);
        return 1;
    }
    return 0;
}

void Init_PointInput(struct PointInput *m, const char *name, int gpio) {
	initMachineBase(&m->machine, name);
    init_io_address(&m->addr, 0, 0, 0, 1, iot_digin, IO_STABLE);
	m->machine.flags &= MASK_PASSIVE; /* not a passive machine */
    m->machine.state = state_PointInput_off;
	m->machine.check_state = ( int(*)(MachineBase*) )PointInput_check_state;
	markPending(&m->machine);
    assert(xSemaphoreGiveRecursive(runtime_mutex) == pdFAIL);
}

struct IOAddress *cw_PointInput_getAddress(struct PointInput *pi) {
    return &pi->addr; 
}

MachineBase *cw_PointInput_To_MachineBase(struct PointInput *p) { return &p->machine; }

int point_input_enter_on(struct PointInput *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
#endif
    m->machine.execute = 0;
    return 1;
}

int point_input_enter_off(struct PointInput *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
#endif
    m->machine.execute = 0;
    return 1;
}


