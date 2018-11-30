#include "runtime.h"
#include "rtscheduler.h"
//#include <stdio.h>
#include <rom/ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#include <pointinput.h>
#include <pointoutput.h>
#include <cw_ANALOGINPUT.h>
#include <cw_ANALOGOUTPUT.h>
#include "cw_Pulse.h"
#include "cw_Ramp.h"
#include "cw_SpeedSelect.h"
#include "cw_DebouncedInput.h"

#include <iointerface.h>

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

static const char* TAG = "setup";

struct cw_Pulse *flasher0;
struct cw_Pulse *flasher1;
struct PointInput *in1;
struct PointOutput *out1;
struct cw_ANALOGINPUT *ain1;
struct cw_ANALOGOUTPUT *aout1;
struct cw_Ramp *ramp0;
struct cw_SpeedSelect *speed_select1;
struct cw_DebouncedInput *d_in1;

//#define point_in1 23 //lolin32
//#define point_out1 22 // lolin32
//#define aout1_pin 33 // lolin32
#define ain1_pin ADC1_CHANNEL_7 // GPIO35
#define point_out1 GPIO_NUM_33 // esp32_gateway LEF
#define point_in1 GPIO_NUM_34 // esp32_gateway BUT1
#define aout1_pin GPIO_NUM_16 // esp32_gateway
#define V_REF 1100

void cwrt_setup() {
    MachineBase *m;

    esp_adc_cal_characteristics_t characteristics;
    memset(&characteristics, 0, sizeof(esp_adc_cal_characteristics_t));
    adc1_config_width(ADC_WIDTH_BIT_12);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, &characteristics);
    adc1_config_channel_atten(ain1_pin, ADC_ATTEN_DB_0);
    gpio_pad_select_gpio(point_in1);
    gpio_set_direction(point_in1, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(point_out1);
    gpio_set_direction(point_out1, GPIO_MODE_OUTPUT);

    // initialise the LEDC driver for analogue output 
    ledc_timer_config_t timer_conf;
    memset(&timer_conf, 0, sizeof(ledc_timer_config_t));
    timer_conf.duty_resolution = LEDC_TIMER_15_BIT;
    timer_conf.freq_hz = 50;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = LEDC_TIMER_0;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf;
    memset(&channel_conf, 0, sizeof(ledc_channel_config_t));
    channel_conf.gpio_num = aout1_pin;
    channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    channel_conf.duty = 0;
    channel_conf.timer_sel = LEDC_TIMER_0;
    channel_conf.hpoint = 9000;
    channel_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_channel_config(&channel_conf);

    out1 = create_PointOutput("O1", point_out1);
    assert(out1);
    m = PointOutput_To_MachineBase(out1);
    assert(m);
    if (m->init) m->init();
    struct IOItem *item_out = IOItem_create(m, PointOutput_getAddress(out1), point_out1);

    in1 = create_PointInput("I1", point_in1);
    assert(in1);
    m = PointInput_To_MachineBase(in1);
    assert(m);
    if (m->init) m->init();
    struct IOItem *item_in = IOItem_create(m, PointInput_getAddress(in1), point_in1);

    ain1 = create_cw_ANALOGINPUT("AIN1", ain1_pin, 0, 0, ADC_CHANNEL_0, 0);
    assert(ain1);
    m = cw_ANALOGINPUT_To_MachineBase(ain1);
    assert(m);
    if (m->init) m->init();
    struct IOItem *item_ain1 = IOItem_create(m, cw_ANALOGINPUT_getAddress(ain1), ain1_pin);
    item_ain1 = item_ain1; // hide compiler warning about unused variable

    aout1 = create_cw_ANALOGOUTPUT("AOUT1", aout1_pin, 0, 0, LEDC_CHANNEL_0);
    assert(aout1);
    m = cw_ANALOGOUTPUT_To_MachineBase(aout1);
    assert(m);
    if (m->init) m->init();
    struct IOItem *item_aout1 = IOItem_create(m, cw_ANALOGOUTPUT_getAddress(aout1), aout1_pin);

    // create clockwork machines
    flasher0 = create_cw_Pulse("F0", out1);
    assert(flasher0);
    m = cw_Pulse_To_MachineBase(flasher0);
    assert(m);
    if (m->init) m->init();
    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);

    ramp0 = create_cw_Ramp("R0", cw_Pulse_To_MachineBase(flasher0), cw_ANALOGOUTPUT_To_MachineBase(aout1));
    assert(ramp0);
    m = cw_Ramp_To_MachineBase(ramp0);
    assert(m);
    if (m->init) m->init();
    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);

    d_in1 = create_cw_DebouncedInput("D1", in1);
    assert(d_in1);
    m = cw_DebouncedInput_To_MachineBase(d_in1);
    assert(m);
    if (m->init) m->init();
    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);

    speed_select1 = create_cw_SpeedSelect("SS1", d_in1, flasher0);
    assert(speed_select1);
    m = cw_SpeedSelect_To_MachineBase(speed_select1);
    assert(m);
    if (m->init) m->init();
    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);

    struct RTIOInterface *interface = RTIOInterface_get();
    while (!interface) {
        taskYIELD();
        interface = RTIOInterface_get();
    }
    ESP_LOGI(TAG,"%lld adding io item", upTime());
    RTIOInterface_add(interface, item_in);
    RTIOInterface_add(interface, item_out);
    RTIOInterface_add(interface, item_aout1);
    createIOMap();
    RTIOInterface_release();

    ESP_LOGI(TAG,"%lld created machine [%d] %s", upTime(), m->id, m->name);

    debug("setup done");
}

