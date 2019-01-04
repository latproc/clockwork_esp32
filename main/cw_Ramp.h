#ifndef __cw_Ramp_h__
#define __cw_Ramp_h__

#include "runtime.h"
#define Value int
struct cw_Ramp_Vars;
struct cw_Ramp_Vars_backup;
struct cw_Ramp {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_clock;
	MachineBase *_output;
	MachineBase *_forward;
	Value direction; // 0
	Value end; // 30000
	Value min; // 5000
	Value start; // 5000
	Value step; // 800
	struct cw_Ramp_Vars *vars;
	struct cw_Ramp_Vars_backup *backup;
};
struct IOAddress *cw_Ramp_getAddress(struct cw_Ramp *p);
struct cw_Ramp *create_cw_Ramp(const char *name, MachineBase *clock, MachineBase *output, MachineBase *forward);
void Init_cw_Ramp(struct cw_Ramp * , const char *name, MachineBase *clock, MachineBase *output, MachineBase *forward);
MachineBase *cw_Ramp_To_MachineBase(struct cw_Ramp *);
#endif
