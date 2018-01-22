#include "runtime.h"
#include "rtscheduler.h"
//#include <stdio.h>
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#include "ex1.h"
#include <pointinput.h>
#include <pointoutput.h>
#include <iointerface.h>

static const char* TAG = "setup";

struct Pulse *flasher0;
struct Pulse *flasher1;
struct PointInput *in1;
struct PointOutput *out1;

#define point_in1 23
#define point_out1 22

void cwrt_setup() {
    MachineBase *m;

    gpio_pad_select_gpio(point_in1);
    gpio_set_direction(point_in1, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(point_out1);
    gpio_set_direction(point_out1, GPIO_MODE_OUTPUT);

    out1 = create_PointOutput("O1", point_out1, 0, 1);
    assert(out1);
    m = PointOutput_To_MachineBase(out1);
    assert(m);
    if (m->init) m->init();
    struct IOItem *item_out = IOItem_create(m, PointOutput_getAddress(out1), point_out1);

    in1 = create_PointInput("I1", point_in1, 0, 0);
    assert(in1);
    m = PointInput_To_MachineBase(in1);
    assert(m);
    if (m->init) m->init();
    struct IOItem *item_in = IOItem_create(m, PointInput_getAddress(in1), point_in1);

    // create clockwork machines
    flasher0 = create_Pulse("F0", out1);
    assert(flasher0);
    m = Pulse_To_MachineBase(flasher0);
    assert(m);
    if (m->init) m->init();
    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);
    
    // 
    // flasher1 = create_Pulse("F1");
    // assert(flasher1);
    // m = Pulse_To_MachineBase(flasher1);
    // assert(m);
    // if (m->init) m->init();
    // ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);
    
    struct RTIOInterface *interface = RTIOInterface_get();
    while (!interface) {
        taskYIELD();
        interface = RTIOInterface_get();
    }
    ESP_LOGI(TAG,"%lld adding io item", upTime());
    RTIOInterface_add(interface, item_in);
    RTIOInterface_add(interface, item_out);
    createIOMap();
    RTIOInterface_release();
    
    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);

    debug("setup done");
}

