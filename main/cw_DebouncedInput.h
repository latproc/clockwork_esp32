#ifndef __cw_DebouncedInput_h__
#define __cw_DebouncedInput_h__

#include "runtime.h"
#define state_cw_DebouncedInput_INIT 1
#define state_cw_DebouncedInput_off 2
#define state_cw_DebouncedInput_on 3
#define Value int
struct cw_DebouncedInput {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_in;
	Value debounce_time; // 100
	Value off_time; // 50
};
struct IOAddress *cw_DebouncedInput_getAddress(struct cw_DebouncedInput *p);
struct cw_DebouncedInput *create_cw_DebouncedInput(const char *name, MachineBase *in);
void Init_cw_DebouncedInput(struct cw_DebouncedInput * , const char *name, MachineBase *in);
MachineBase *cw_DebouncedInput_To_MachineBase(struct cw_DebouncedInput *);
#endif
