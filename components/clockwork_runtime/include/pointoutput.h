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

#define state_PointOutput_off 0
#define state_PointOutput_on 1

struct PointOutput;
struct PointOutput *create_PointOutput(const char *name, int gpio, uint8_t offset, uint8_t pos);
void Init_PointOutput(struct PointOutput *, const char *name, int gpio, uint8_t offset, uint8_t pos);
MachineBase *PointOutput_To_MachineBase(struct PointOutput *);
struct IOAddress *PointOutput_getAddress(struct PointOutput *);
void turnOn(struct PointOutput *output);
void turnOff(struct PointOutput *output);

#endif
