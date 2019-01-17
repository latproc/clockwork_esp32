#ifndef __cw_LEDSTRIP_h__
#define __cw_LEDSTRIP_h__

struct cw_DIGITALLED;
#include "runtime.h"
#define Value int
struct cw_LEDSTRIP_Vars;
struct cw_LEDSTRIP_Vars_backup;
struct private_strand;
struct cw_LEDSTRIP {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_out;
	MachineBase *_led_type;
	Value channel; // 1
	Value max_output; // 32
	Value num_pixels; // 8
	Value pin; // "out"
	struct list_head leds;
	struct private_strand *strand;
	struct cw_LEDSTRIP_Vars *vars;
	struct cw_LEDSTRIP_Vars_backup *backup;
};
//struct IOAddress *cw_LEDSTRIP_getAddress(struct cw_LEDSTRIP *p);
struct cw_LEDSTRIP *create_cw_LEDSTRIP(const char *name, MachineBase *out, MachineBase *led_type);
void Init_cw_LEDSTRIP(struct cw_LEDSTRIP * , const char *name, MachineBase *out, MachineBase *led_type);
MachineBase *cw_LEDSTRIP_To_MachineBase(struct cw_LEDSTRIP *);
void add_led_to_strip(struct cw_LEDSTRIP *strip, struct cw_DIGITALLED *led);
#endif
