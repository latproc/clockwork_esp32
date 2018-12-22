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

#define state_PointInput_off 2
#define state_PointInput_on 3

struct PointInput;
struct PointInput *create_cw_PointInput(const char *name, int gpio);
void Init_PointInput(struct PointInput *, const char *name, int gpio);
MachineBase *cw_PointInput_To_MachineBase(struct PointInput *);
struct IOAddress *cw_PointInput_getAddress(struct PointInput *);

#endif
