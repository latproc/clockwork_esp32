#ifndef __cw_SpeedSelect_h__
#define __cw_SpeedSelect_h__

#include "runtime.h"
#define Value int
struct cw_SpeedSelect_Vars;
struct cw_SpeedSelect_Vars_backup;
struct cw_SpeedSelect {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_button;
	MachineBase *_pulser;
	struct cw_SpeedSelect_Vars *vars;
	struct cw_SpeedSelect_Vars_backup *backup;
};
struct IOAddress *cw_SpeedSelect_getAddress(struct cw_SpeedSelect *p);
struct cw_SpeedSelect *create_cw_SpeedSelect(const char *name, MachineBase *button, MachineBase *pulser);
void Init_cw_SpeedSelect(struct cw_SpeedSelect * , const char *name, MachineBase *button, MachineBase *pulser);
MachineBase *cw_SpeedSelect_To_MachineBase(struct cw_SpeedSelect *);
#endif
