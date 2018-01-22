#ifndef __EX1_H__
#define __EX1_H__

struct Pulse;
struct PointOutput;
struct Pulse *create_Pulse(const char *name, struct PointOutput *out);
void Init_Pulse(struct Pulse *, const char *name, struct PointOutput *out);
MachineBase *Pulse_To_MachineBase(struct Pulse *);

#endif
