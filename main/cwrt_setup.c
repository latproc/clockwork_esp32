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
#include "cw_SYSTEMSETTINGS.h"
#include <cw_MQTTBROKER.h>
#include <cw_MQTTPUBLISHER.h>
#include <cw_MQTTSUBSCRIBER.h>
#include <cw_DIGITALLEDS.h>

#include <iointerface.h>

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

#include "project.h"

static const char* TAG = "setup";

#define V_REF 1100

void cwrt_setup() {
    MachineBase *m;

    esp_adc_cal_characteristics_t characteristics;
    memset(&characteristics, 0, sizeof(esp_adc_cal_characteristics_t));
    adc1_config_width(ADC_WIDTH_BIT_12);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, &characteristics);

    // initialise the LEDC driver for pwm output 
    ledc_timer_config_t timer_conf;
    memset(&timer_conf, 0, sizeof(ledc_timer_config_t));
    timer_conf.duty_resolution = LEDC_TIMER_15_BIT;
    timer_conf.freq_hz = 50;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = LEDC_TIMER_0;
    ledc_timer_config(&timer_conf);

    struct cw_SYSTEMSETTINGS *system_settings = create_cw_SYSTEMSETTINGS("SYSTEM");
    {
        struct MachineBase *m = &system_settings->machine;
        if (m->init) m->init();
    }

    struct RTIOInterface *interface = RTIOInterface_get();
    while (!interface) {
    taskYIELD();
    interface = RTIOInterface_get();
    }

#include "cw_setup.inc"

    createIOMap();
    RTIOInterface_release();

    debug("setup done");
}

