#ifndef __cw_Ramp_h__
#define __cw_Ramp_h__

#include "runtime.h"
#define state_cw_Ramp_INIT 1
#define state_cw_Ramp_top 11
#define state_cw_Ramp_bottom 7
#define state_cw_Ramp_rising 9
#define state_cw_Ramp_falling 8
#define state_cw_Ramp_stopped 10
#define Value int
struct cw_Ramp {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_clock;
	MachineBase *_output;
	Value VALUE; // 0
	Value direction; // 0
	Value end; // 30000
	Value start; // 1000
	Value step; // 800
};
struct IOAddress *cw_Ramp_getAddress(struct cw_Ramp *p);
struct cw_Ramp *create_cw_Ramp(const char *name, MachineBase *clock, MachineBase *output);
void Init_cw_Ramp(struct cw_Ramp * , const char *name, MachineBase *clock, MachineBase *output);
MachineBase *cw_Ramp_To_MachineBase(struct cw_Ramp *);
#endif
