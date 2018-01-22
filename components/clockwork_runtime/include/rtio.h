#ifndef __rtio_h__
#define __rtio_h__

#include <inttypes.h>

typedef enum { IO_STABLE, IO_PENDING, IO_DONE } IOStatus;

union IOValue {
	uint8_t u8;
	int8_t s8;
	uint16_t u16;
	int16_t s16;
	uint32_t u32;
	int32_t s32;
};

typedef enum { iot_digin, iot_digout } IOType;

struct IOAddress {
	uint8_t module_position;
    uint8_t io_offset; // byte offset
    uint8_t io_bitpos; // bit offset within byte
	uint8_t bitlen; // length in bits
    union IOValue value;
    IOType io_type;
    IOStatus status;
};

void rt_set_io_bit(struct IOAddress *, uint8_t val);
void rt_set_io_byte(struct IOAddress *, uint8_t val);
void rt_set_io_uint16(struct IOAddress *, uint16_t val);
void rt_set_io_uint32(struct IOAddress *, uint32_t val);

uint8_t rt_get_io_bit(struct IOAddress *a);
uint8_t rt_get_io_byte(struct IOAddress *a);
uint16_t rt_get_io_uint16(struct IOAddress *a);
uint32_t rt_get_io_uint32(struct IOAddress *a);

#endif
