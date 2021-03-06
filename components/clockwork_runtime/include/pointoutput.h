//
//  PointOutput.h
//  Project: cwrt
//
//	All rights reserved. Use of this source code is governed by the
//	3-clause BSD License in LICENSE.txt.

#ifndef __pointoutput_h__
#define __pointoutput_h__

#include "runtime.h"
#include "rtio.h"

#define state_PointOutput_off 2
#define state_PointOutput_on 3

struct PointOutput {
	MachineBase machine;
    int gpio_pin;
    int level;
    struct IOAddress addr;
};

struct PointOutput *create_cw_PointOutput(const char *name, int gpio, int level);
void Init_PointOutput(struct PointOutput *, const char *name, int gpio, int level);
MachineBase *cw_PointOutput_To_MachineBase(struct PointOutput *);
struct IOAddress *cw_PointOutput_getAddress(struct PointOutput *);
void turnOn(struct PointOutput *output);
void turnOff(struct PointOutput *output);

#endif
