#ifndef __Pulse_h__
#define __Pulse_h__

#include "runtime.h"
#define state_cw_Pulse_INIT 1
#define state_cw_Pulse_on 3
#define state_cw_Pulse_off 2
struct cw_Pulse;
struct IOAddress *cw_Pulse_getAddress(struct cw_Pulse *p);
struct cw_Pulse *create_cw_Pulse(const char *name, MachineBase *out);
void Init_cw_Pulse(struct cw_Pulse * , const char *name, MachineBase *out);
MachineBase *cw_Pulse_To_MachineBase(struct cw_Pulse *);
#endif
