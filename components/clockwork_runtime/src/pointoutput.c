//
//  PointOutput.cpp
//  Project: cwrt
//
//	All rights reserved. Use of this source code is governed by the
//	3-clause BSD License in LICENSE.txt.

#include "pointoutput.h"
#include <esp_log.h>
#include "rtio.h"
#include "iointerface.h"

static const char* TAG = "PointOutput";

#define DEBUG_LOG 0

struct PointOutput {
	MachineBase machine;
    int gpio_pin;
    struct IOAddress addr;
};

struct PointOutput *create_PointOutput(const char *name, int gpio, uint8_t offset, uint8_t pos) {
    struct PointOutput *p = (struct PointOutput *)malloc(sizeof(struct PointOutput));
	Init_PointOutput(p, name, gpio, offset, pos);
	return p;
}

int PointOutput_check_state(struct PointOutput *m) {
    if (m->addr.status == IO_DONE) { // state change
        m->addr.status = IO_STABLE;
        uint8_t val = rt_get_io_bit(&m->addr);
#if DEBUG_LOG
        ESP_LOGI(TAG,"io value is now %d (%d)", m->addr.value.u8, val);
#endif

        if (val) {
            m->machine.state = state_PointOutput_on;
            ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
        }
        else {
            m->machine.state = state_PointOutput_off;
            ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
        }
    }
    return 1;
}

void Init_PointOutput(struct PointOutput *m, const char *name, int gpio, uint8_t offset, uint8_t pos) {
	initMachineBase(&m->machine, name);
    m->addr.module_position = 0;
    m->addr.io_offset = offset;
    m->addr.io_bitpos = pos;
    m->addr.bitlen = 1;
    m->addr.status = IO_STABLE;
    m->addr.io_type = iot_digout;
	m->machine.flags &= MASK_PASSIVE; /* not a passive machine */
    m->machine.state = state_PointOutput_off;
	m->machine.check_state = ( int(*)(MachineBase*) )PointOutput_check_state;
	markPending(&m->machine);
    assert(xSemaphoreGiveRecursive(runtime_mutex) == pdFAIL);
}

struct IOAddress *PointOutput_getAddress(struct PointOutput *pi) {
    return &pi->addr; 
}

MachineBase *PointOutput_To_MachineBase(struct PointOutput *p) { return &p->machine; }

int point_output_enter_on(struct PointOutput *m, ccrContParam) {
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
    m->machine.execute = 0;
    return 1;
}

int point_output_enter_off(struct PointOutput *m, ccrContParam) {
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
    m->machine.execute = 0;
    return 1;
}

void turnOn(struct PointOutput *output) {
    rt_set_io_bit(&output->addr, 1);
    output->addr.status = IO_PENDING;
}

void turnOff(struct PointOutput *output) {
    rt_set_io_bit(&output->addr, 0);
    output->addr.status = IO_PENDING;
}



