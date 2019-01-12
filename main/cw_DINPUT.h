#ifndef __cw_DINPUT_h__
#define __cw_DINPUT_h__

#include "runtime.h"
#define Value int
struct cw_DINPUT_Vars;
struct cw_DINPUT_Vars_backup;
struct cw_DINPUT {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_Input;
	Value stable; // 50
	struct cw_DINPUT_Vars *vars;
	struct cw_DINPUT_Vars_backup *backup;
};
struct IOAddress *cw_DINPUT_getAddress(struct cw_DINPUT *p);
struct cw_DINPUT *create_cw_DINPUT(const char *name, MachineBase *Input);
void Init_cw_DINPUT(struct cw_DINPUT * , const char *name, MachineBase *Input);
MachineBase *cw_DINPUT_To_MachineBase(struct cw_DINPUT *);
#endif
