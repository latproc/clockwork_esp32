#ifndef __cw_DIGITALLED_h__
#define __cw_DIGITALLED_h__

#include "runtime.h"
#define Value int
struct cw_DIGITALLED_Vars;
struct cw_DIGITALLED_Vars_backup;
struct cw_DIGITALLED {
	MachineBase machine;
	int gpio_pin;
	struct IOAddress addr;
	MachineBase *_strip;
	Value position;
	Value b; // 0
	Value g; // 0
	Value max; // 32
	Value r; // 0
	struct cw_DIGITALLED_Vars *vars;
	struct cw_DIGITALLED_Vars_backup *backup;
};
struct IOAddress *cw_DIGITALLED_getAddress(struct cw_DIGITALLED *p);
struct cw_DIGITALLED *create_cw_DIGITALLED(const char *name, MachineBase *strip, int position);
void Init_cw_DIGITALLED(struct cw_DIGITALLED * , const char *name, MachineBase *strip, int position);
MachineBase *cw_DIGITALLED_To_MachineBase(struct cw_DIGITALLED *);
#endif
