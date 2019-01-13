#ifndef __cw_DIGITALLEDS_h__
#define __cw_DIGITALLEDS_h__

#include "runtime.h"
#define Value int
struct cw_DIGITALLEDS_Vars;
struct cw_DIGITALLEDS_Vars_backup;
struct cw_DIGITALLEDS {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_out;
	Value channel; // 1
	Value max_output; // 32
	Value num_pixels; // 8
	Value pin; // "out"
	struct cw_DIGITALLEDS_Vars *vars;
	struct cw_DIGITALLEDS_Vars_backup *backup;
};
struct IOAddress *cw_DIGITALLEDS_getAddress(struct cw_DIGITALLEDS *p);
struct cw_DIGITALLEDS *create_cw_DIGITALLEDS(const char *name, MachineBase *out);
void Init_cw_DIGITALLEDS(struct cw_DIGITALLEDS * , const char *name, MachineBase *out);
MachineBase *cw_DIGITALLEDS_To_MachineBase(struct cw_DIGITALLEDS *);
#endif
