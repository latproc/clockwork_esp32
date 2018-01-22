#include "rtio.h"
#include <esp_log.h>

extern uint8_t *io_map;

static const char* TAG = "RTIO";

void rt_set_io_bit(struct IOAddress *a, uint8_t val) {
	ESP_LOGI(TAG, "setting bit %d:%d to %d",a->io_offset, a->io_bitpos, val);
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
