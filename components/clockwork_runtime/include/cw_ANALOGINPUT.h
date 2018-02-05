#ifndef __ANALOGINPUT_h__
#define __ANALOGINPUT_h__

#include "runtime.h"
#define state_cw_INIT 0
#define state_cw_stable 1
#define state_cw_unstable 2
struct cw_ANALOGINPUT;
struct IOAddress *cw_ANALOGINPUT_getAddress(struct cw_ANALOGINPUT *p);
int cw_ANALOGINPUT_getChannel(struct cw_ANALOGINPUT *p);
struct cw_ANALOGINPUT *create_cw_ANALOGINPUT(const char *name, int pin, MachineBase *module, int offset, int channel, MachineBase *filter_settings);
void Init_cw_ANALOGINPUT(struct cw_ANALOGINPUT * , const char *name, int pin, MachineBase *module, int offset, int channel, MachineBase *filter_settings);
MachineBase *cw_ANALOGINPUT_To_MachineBase(struct cw_ANALOGINPUT *);
#endif
