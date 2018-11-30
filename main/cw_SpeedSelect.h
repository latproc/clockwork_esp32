#ifndef __SpeedSelect_h__
#define __SpeedSelect_h__

#include "runtime.h"
#define state_cw_SpeedSelect_INIT 1
#define state_cw_SpeedSelect_fast 11
#define state_cw_SpeedSelect_slow 12
struct cw_SpeedSelect;
struct IOAddress *cw_SpeedSelect_getAddress(struct cw_SpeedSelect *p);
struct cw_SpeedSelect *create_cw_SpeedSelect(const char *name, MachineBase *button, MachineBase *pulser);
void Init_cw_SpeedSelect(struct cw_SpeedSelect * , const char *name, MachineBase *button, MachineBase *pulser);
MachineBase *cw_SpeedSelect_To_MachineBase(struct cw_SpeedSelect *);
#endif
