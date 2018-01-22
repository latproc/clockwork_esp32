//
//  PointInput.cpp
//  Project: cwrt
//
//	All rights reserved. Use of this source code is governed by the
//	3-clause BSD License in LICENSE.txt.

#include "pointinput.h"
#include <esp_log.h>
#include "rtio.h"
#include "iointerface.h"

static const char* TAG = "PointInput";

#define DEBUG_LOG 0

struct PointInput {
	MachineBase machine;
    int gpio_pin;
    struct IOAddress addr;
};

struct PointInput *create_PointInput(const char *name, int gpio, uint8_t offset, uint8_t pos) {
    struct PointInput *p = (struct PointInput *)malloc(sizeof(struct PointInput));
	Init_PointInput(p, name, gpio, offset, pos);
	return p;
}

int PointInput_check_state(struct PointInput *m) {
    if (m->addr.status == IO_DONE) { // state change
        m->addr.status = IO_STABLE;
        uint8_t val = rt_get_io_bit(&m->addr);
#if DEBUG_LOG
        ESP_LOGI(TAG,"io value is now %d (%d)", m->addr.value.u8, val);
#endif

        if (val) {
            m->machine.state = state_PointInput_on;
            ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
        }
        else {
            m->machine.state = state_PointInput_off;
            ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
        }
    }
    return 1;
}

void Init_PointInput(struct PointInput *m, const char *name, int gpio, uint8_t offset, uint8_t pos) {
	initMachineBase(&m->machine, name);
    m->addr.module_position = 0;
    m->addr.io_offset = offset;
    m->addr.io_bitpos = pos;
    m->addr.bitlen = 1;
    m->addr.status = IO_STABLE;
    m->addr.io_type = iot_digin;
	m->machine.flags &= MASK_PASSIVE; /* not a passive machine */
    m->machine.state = state_PointInput_off;
	m->machine.check_state = ( int(*)(MachineBase*) )PointInput_check_state;
	markPending(&m->machine);
    assert(xSemaphoreGiveRecursive(runtime_mutex) == pdFAIL);
}

struct IOAddress *PointInput_getAddress(struct PointInput *pi) {
    return &pi->addr; 
}

MachineBase *PointInput_To_MachineBase(struct PointInput *p) { return &p->machine; }

int point_input_enter_on(struct PointInput *m, ccrContParam) {
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
    m->machine.execute = 0;
    return 1;
}

int point_input_enter_off(struct PointInput *m, ccrContParam) {
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
    m->machine.execute = 0;
    return 1;
}


