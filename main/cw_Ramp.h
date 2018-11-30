#ifndef __Ramp_h__
#define __Ramp_h__

#include "runtime.h"
#define state_cw_Ramp_INIT 1
#define state_cw_Ramp_top 10
#define state_cw_Ramp_bottom 6
#define state_cw_Ramp_rising 8
#define state_cw_Ramp_falling 7
#define state_cw_Ramp_stopped 9
struct cw_Ramp;
struct IOAddress *cw_Ramp_getAddress(struct cw_Ramp *p);
struct cw_Ramp *create_cw_Ramp(const char *name, MachineBase *clock, MachineBase *output);
void Init_cw_Ramp(struct cw_Ramp * , const char *name, MachineBase *clock, MachineBase *output);
MachineBase *cw_Ramp_To_MachineBase(struct cw_Ramp *);
#endif
