#ifndef __cw_Pulse_h__
#define __cw_Pulse_h__

#include "runtime.h"
#define Value int
struct cw_Pulse_Vars;
struct cw_Pulse_Vars_backup;
struct cw_Pulse {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	Value delay; // 100
	struct cw_Pulse_Vars *vars;
	struct cw_Pulse_Vars_backup *backup;
};
struct IOAddress *cw_Pulse_getAddress(struct cw_Pulse *p);
struct cw_Pulse *create_cw_Pulse(const char *name);
void Init_cw_Pulse(struct cw_Pulse * , const char *name);
MachineBase *cw_Pulse_To_MachineBase(struct cw_Pulse *);
#endif
