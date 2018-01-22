#ifndef __FLAG_h__
#define __FLAG_h__

#include "runtime.h"
#define state_INIT_0
#define state_on_1
#define state_off_2
struct cw_FLAG;
struct FLAG *create_FLAG();
void Init_FLAG(struct FLAG * );
MachineBase *FLAG_To_MachineBase(struct FLAG *);
#endif
