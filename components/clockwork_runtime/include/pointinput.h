//
//  PointInput.h
//  Project: cwrt
//
//	All rights reserved. Use of this source code is governed by the
//	3-clause BSD License in LICENSE.txt.

#ifndef __pointinput_h__
#define __pointinput_h__

#include "runtime.h"
#include "rtio.h"

#define state_PointInput_off 0
#define state_PointInput_on 1

struct PointInput;
struct PointInput *create_PointInput(const char *name, int gpio);
void Init_PointInput(struct PointInput *, const char *name, int gpio);
MachineBase *PointInput_To_MachineBase(struct PointInput *);
struct IOAddress *PointInput_getAddress(struct PointInput *);

#endif
