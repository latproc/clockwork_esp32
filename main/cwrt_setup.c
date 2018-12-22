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
//struct PointInput *in1;
//struct PointOutput *out1;
struct cw_ANALOGINPUT *ain1;
struct cw_ANALOGOUTPUT *aout1;
struct cw_Ramp *ramp0;
struct cw_SpeedSelect *speed_select1;
struct cw_DebouncedInput *d_in1;

//#define point_in1 23 //lolin32
//#define point_out1 22 // lolin32
//#define aout1_pin 33 // lolin32
#define ain1_pin ADC1_CHANNEL_7 // GPIO35
//#define point_out1 GPIO_NUM_33 // esp32_gateway LEF
//#define point_in1 GPIO_NUM_34 // esp32_gateway BUT1
#define aout1_pin GPIO_NUM_16 // esp32_gateway
#define V_REF 1100

void cwrt_setup() {
    MachineBase *m;

    esp_adc_cal_characteristics_t characteristics;
    memset(&characteristics, 0, sizeof(esp_adc_cal_characteristics_t));
    adc1_config_width(ADC_WIDTH_BIT_12);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, &characteristics);
    adc1_config_channel_atten(ain1_pin, ADC_ATTEN_DB_0);

    // initialise the LEDC driver for pwm output 
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

#define cw_OLIMEX_GATEWAY32_LED 33
#define cw_OLIMEX_GATEWAY32_BUT1 34
#define cw_OLIMEX_GATEWAY32_GPIO16 16
struct RTIOInterface *interface = RTIOInterface_get();
while (!interface) {
  taskYIELD();
  interface = RTIOInterface_get();
}
struct PointInput *cw_inst_button = create_cw_PointInput("button", cw_OLIMEX_GATEWAY32_BUT1);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_BUT1);
gpio_set_direction(cw_OLIMEX_GATEWAY32_BUT1, GPIO_MODE_INPUT);
{
	struct MachineBase *m = cw_PointInput_To_MachineBase(cw_inst_button);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_button = IOItem_create(m, cw_PointInput_getAddress(cw_inst_button), cw_OLIMEX_GATEWAY32_BUT1);
	RTIOInterface_add(interface, item_cw_inst_button);
}
struct PointOutput *cw_inst_led = create_cw_PointOutput("led", cw_OLIMEX_GATEWAY32_LED);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_LED);
gpio_set_direction(cw_OLIMEX_GATEWAY32_LED, GPIO_MODE_OUTPUT);
{
	struct MachineBase *m = cw_PointOutput_To_MachineBase(cw_inst_led);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_led = IOItem_create(m, cw_PointOutput_getAddress(cw_inst_led), cw_OLIMEX_GATEWAY32_LED);
	RTIOInterface_add(interface, item_cw_inst_led);
}
struct ANALOGOUTPUT *cw_inst_aout = create_cw_ANALOGOUTPUT("aout", cw_OLIMEX_GATEWAY32_GPIO16, 0, 0, LEDC_CHANNEL_0);
{
	struct MachineBase *m = cw_ANALOGOUTPUT_To_MachineBase(cw_inst_aout);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_aout = IOItem_create(m, cw_ANALOGOUTPUT_getAddress(cw_inst_aout), cw_OLIMEX_GATEWAY32_GPIO16);
	RTIOInterface_add(interface, item_cw_inst_aout);
}
struct Pulse *cw_inst_pulser = create_cw_Pulse("pulser", cw_inst_led);
{
	struct MachineBase *m = cw_Pulse_To_MachineBase(cw_inst_pulser);
	if (m->init) m->init();
}
struct Ramp *cw_inst_ramp = create_cw_Ramp("ramp", cw_inst_pulser, cw_inst_aout);
{
	struct MachineBase *m = cw_Ramp_To_MachineBase(cw_inst_ramp);
	if (m->init) m->init();
}
struct DebouncedInput *cw_inst_d_button = create_cw_DebouncedInput("d_button", cw_inst_button);
{
	struct MachineBase *m = cw_DebouncedInput_To_MachineBase(cw_inst_d_button);
	if (m->init) m->init();
}
struct SpeedSelect *cw_inst_speed_select = create_cw_SpeedSelect("speed_select", cw_inst_d_button, cw_inst_pulser);
{
	struct MachineBase *m = cw_SpeedSelect_To_MachineBase(cw_inst_speed_select);
	if (m->init) m->init();
}

    createIOMap();
    RTIOInterface_release();

    debug("setup done");
}

