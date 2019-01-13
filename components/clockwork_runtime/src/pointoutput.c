//
//  PointOutput.cpp
//  Project: cwrt
//
//	All rights reserved. Use of this source code is governed by the
//	3-clause BSD License in LICENSE.txt.

#include "base_includes.h"
#include "pointoutput.h"

#define DEBUG_LOG 0
#if DEBUG_LOG
static const char* TAG = "PointOutput";
#endif

int *cw_OUTPUT_lookup(struct PointOutput *m, int symbol) {
	if (symbol == sym_pin) return &m->gpio_pin;
	return 0;
}
MachineBase *cw_OUTPUT_lookup_machine(MachineBase *m, int symbol) {
	return 0;
}

struct PointOutput *create_cw_PointOutput(const char *name, int gpio, int level) {
    struct PointOutput *p = (struct PointOutput *)malloc(sizeof(struct PointOutput));
	Init_PointOutput(p, name, gpio, level);
	return p;
}

int point_output_enter_on(struct PointOutput *m, ccrContParam);
int point_output_enter_off(struct PointOutput *m, ccrContParam);
int point_output_handle_message(struct MachineBase *obj, struct MachineBase *source, int message) {
#if DEBUG_LOG
    ESP_LOGI(TAG,"%lld [%d] handling message [%d]", upTime(), obj->id, message);
#endif
	if (message == cw_message_turnOn) { 
		turnOn( (struct PointOutput *)obj);
    }
    else if (message == cw_message_turnOff)
		turnOff( (struct PointOutput *)obj);
	return 1;
}
int PointOutput_check_state(struct PointOutput *m) {
    if (m->addr.status == IO_DONE) { // state change
        m->addr.status = IO_STABLE;
        uint8_t val = rt_get_io_bit(&m->addr);
#if DEBUG_LOG
        ESP_LOGI(TAG,"io value is now %d (%d)", m->addr.value.u8, val);
#endif
        if (val)
            changeMachineState(cw_PointOutput_To_MachineBase(m), state_PointOutput_on, (enter_func) point_output_enter_on);
        else
            changeMachineState(cw_PointOutput_To_MachineBase(m), state_PointOutput_off, (enter_func) point_output_enter_off);
        return 1;
    }
    return 0;
}

void Init_PointOutput(struct PointOutput *m, const char *name, int gpio, int level) {
	initMachineBase(&m->machine, name);
    m->machine.class_name = "OUTPUT";
    m->gpio_pin = gpio;
    gpio_pad_select_gpio(gpio);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, level);
    init_io_address(&m->addr, 0, 0, 0, 1, iot_digout, IO_STABLE);
	m->machine.flags &= MASK_PASSIVE; /* not a passive machine */
    m->machine.state = state_PointOutput_off;
	m->machine.check_state = ( int(*)(MachineBase*) )PointOutput_check_state;
    m->machine.handle = (message_func)point_output_handle_message;
    m->machine.lookup = (lookup_func)cw_OUTPUT_lookup;
    m->machine.lookup_machine = (lookup_machine_func)cw_OUTPUT_lookup_machine;
	markPending(&m->machine);
    assert(xSemaphoreGiveRecursive(runtime_mutex) == pdFAIL);
}

struct IOAddress *cw_PointOutput_getAddress(struct PointOutput *pi) {
    return &pi->addr; 
}

MachineBase *cw_PointOutput_To_MachineBase(struct PointOutput *p) { return &p->machine; }

int point_output_enter_on(struct PointOutput *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "on");
#endif
    m->machine.execute = 0;
    return 1;
}

int point_output_enter_off(struct PointOutput *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "%lld [%d] %s",upTime(), m->machine.id, "off");
#endif
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



