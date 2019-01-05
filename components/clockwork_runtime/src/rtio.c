#include "rtio.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"

#define DEBUG_LOG 0
//static const char* TAG = "RTIO";

extern uint8_t *io_map;

void init_io_address(struct IOAddress *addr, uint8_t module_position, uint8_t offset, uint8_t pos, uint8_t bitlen, IOType io_type, IOStatus status) {
    addr->module_position = module_position;
    addr->io_offset = offset;
    addr->io_bitpos = pos;
    addr->bitlen = bitlen;
    addr->io_type = io_type;
    addr->status = status;
}

void rt_set_io_bit(struct IOAddress *a, uint8_t val) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "setting bit %d:%d to %d",a->io_offset, a->io_bitpos, val);
#endif
    if (val)
        a->value.u8 |= (1 << a->io_bitpos);
    else
        a->value.u8 &= -1 - (1 << a->io_bitpos);
    assert(rt_get_io_bit(a) == val);
}

void rt_set_io_byte(struct IOAddress *a, uint8_t val) {
    a->value.u8 = val;
}

void rt_set_io_uint16(struct IOAddress *a, uint16_t val) {
    a->value.u16 = val;
}

void rt_set_io_uint32(struct IOAddress *a, uint32_t val) {
    a->value.u32 = val;
}

uint8_t rt_get_io_bit(struct IOAddress *a) {
    //union IOValue *value = (union IOValue *)(io_map + a->io_offset);
    uint8_t res = (a->value.u8 & (1 << a->io_bitpos)) >> a->io_bitpos;
    return res;
}

uint8_t rt_get_io_byte(struct IOAddress *a) {
    //union IOValue *value = (union IOValue *)(io_map + a->io_offset);
    return a->value.u8;
}

uint16_t rt_get_io_uint16(struct IOAddress *a) {
    //union IOValue *value = (union IOValue *)(io_map + a->io_offset);
    return a->value.u16;
}

uint32_t rt_get_io_uint32(struct IOAddress *a) {
    //union IOValue *value = (union IOValue *)(io_map + a->io_offset);
    return a->value.u32;
}

void setup_pwm_gpio(int channel_num, int gpio_num, int duty) {
    ledc_channel_config_t ledc_conf;
    ledc_conf.channel = channel_num;
    ledc_conf.duty = duty;
    ledc_conf.gpio_num = gpio_num;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_conf.timer_sel = LEDC_TIMER_0;
    ledc_conf.hpoint = 1000;
    ledc_channel_config(&ledc_conf);
}
