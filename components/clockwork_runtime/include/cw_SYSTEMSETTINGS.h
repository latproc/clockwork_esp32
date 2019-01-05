#ifndef __cw_SYSTEMSETTINGS_h__
#define __cw_SYSTEMSETTINGS_h__

#include "runtime.h"
#define Value int
struct cw_SYSTEMSETTINGS_Vars;
struct cw_SYSTEMSETTINGS_Vars_backup;
struct cw_SYSTEMSETTINGS {
	MachineBase machine;
	Value CYCLE_DELAY; // 2000
	const char *HOST; // Mobook.local
	const char *INFO; // Clockwork host
	Value POLLING_DELAY; // 2000
	Value VERSION; // 0.9
	struct cw_SYSTEMSETTINGS_Vars *vars;
	struct cw_SYSTEMSETTINGS_Vars_backup *backup;
};
struct cw_SYSTEMSETTINGS *create_cw_SYSTEMSETTINGS(const char *name);
void Init_cw_SYSTEMSETTINGS(struct cw_SYSTEMSETTINGS * , const char *name);
#endif
