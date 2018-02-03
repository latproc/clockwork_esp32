#ifndef __ANALOGOUTPUT_h__
#define __ANALOGOUTPUT_h__

#include "runtime.h"
#define state_cw_INIT 0
#define state_cw_stable 1
#define state_cw_unstable 2
struct cw_ANALOGOUTPUT;
struct cw_ANALOGOUTPUT *create_cw_ANALOGOUTPUT(const char *name, int pin, MachineBase *module, int offset, int channel);
void Init_cw_ANALOGOUTPUT(struct cw_ANALOGOUTPUT * , const char *name, int pin, MachineBase *module, int offset, int channel);
struct IOAddress *cw_ANALOGOUTPUT_getAddress(struct cw_ANALOGOUTPUT *p);
MachineBase *cw_ANALOGOUTPUT_To_MachineBase(struct cw_ANALOGOUTPUT *);
void cw_ANALOGOUTPUT_set_value (struct cw_ANALOGOUTPUT *output, uint16_t value);
int cw_ANALOGOUTPUT_get_channel(struct cw_ANALOGOUTPUT *output);
#endif
