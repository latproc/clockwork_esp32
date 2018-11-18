#ifndef __EX1_H__
#define __EX1_H__

#define state_cw_Pulse_INIT 1
#define state_cw_Pulse_on 3
#define state_cw_Pulse_off 2

struct Pulse;
struct PointOutput;
struct Pulse *create_Pulse(const char *name, struct PointOutput *out);
void Init_Pulse(struct Pulse *, const char *name, struct PointOutput *out);
MachineBase *Pulse_To_MachineBase(struct Pulse *);

#endif
