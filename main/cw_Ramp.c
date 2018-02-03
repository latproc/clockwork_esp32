
#include "base_includes.h"
#include "cw_Ramp.h"
//static const char* TAG = "Ramp";
#define DEBUG_LOG 0
#define Value int
struct cw_Ramp {
MachineBase machine;
int gpio_pin;
struct IOAddress addr;
MachineBase *_clock;
MachineBase *_output;
Value VALUE; // 0
Value direction; // 0
Value end; // 10000
Value start; // 1000
Value step; // 10
};
int cw_Ramp_check_state(struct cw_Ramp *m);
struct cw_Ramp *create_cw_Ramp(const char *name, MachineBase *clock, MachineBase *output) {
struct cw_Ramp *p = (struct cw_Ramp *)malloc(sizeof(struct cw_Ramp));
Init_cw_Ramp(p, name, clock, output);
return p;
}
void Init_cw_Ramp(struct cw_Ramp *m, const char *name, MachineBase *clock, MachineBase *output) {
initMachineBase(&m->machine, name);
init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
m->_clock = clock;
m->_output = output;
m->VALUE = 0;
m->direction = 0;
m->end = 10000;
m->start = 1000;
m->step = 10;
m->machine.state = 0;
m->machine.check_state = ( int(*)(MachineBase*) )cw_Ramp_check_state;
markPending(&m->machine);
}
struct IOAddress *cw_Ramp_getAddress(struct cw_Ramp *p) {
  return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_Ramp_To_MachineBase(struct cw_Ramp *p) { return &p->machine; }
int Ramp_enter_INIT(struct cw_Ramp *m, ccrContParam) {// INIT 
  m->VALUE =  m->start;
  m->direction =  1;
  m->machine.execute = 0; 
  return 1;
}
int cw_Ramp_enter_top(struct cw_Ramp *m, ccrContParam) {// top 
  m->VALUE =  m->end;
  m->direction =  -1;
  m->machine.execute = 0; 
  return 1; 
}
int cw_Ramp_enter_bottom(struct cw_Ramp *m, ccrContParam) {// bottom 
  m->VALUE =  m->start;
  m->direction =  1;
  m->machine.execute = 0; 
  return 1;
}
int cw_Ramp_enter_rising(struct cw_Ramp *m, ccrContParam) {m->machine.execute = 0; return 1; }
int cw_Ramp_enter_falling(struct cw_Ramp *m, ccrContParam) {m->machine.execute = 0; return 1; }
int cw_Ramp_enter_stopped(struct cw_Ramp *m, ccrContParam) {m->machine.execute = 0; return 1; }
int cw_Ramp_check_state(struct cw_Ramp *m) {
  int new_state = 0; enter_func new_state_enter = 0;
  if ((( m->VALUE >=  m->end) && ( m->direction >  0))) {
    new_state = state_cw_Ramp_top;
    new_state_enter = (enter_func)cw_Ramp_enter_top;
  }
  else
  if ((( m->VALUE <=  m->start) && ( m->direction <  0))) {
    new_state = state_cw_Ramp_bottom;
    new_state_enter = (enter_func)cw_Ramp_enter_bottom;
  }
  else
  if ((( m->VALUE < m->end) && ( m->direction >  0))) {
    new_state = state_cw_Ramp_rising;
    new_state_enter = (enter_func)cw_Ramp_enter_rising;
  }
  else
  if ((( m->VALUE > m->start) && ( m->direction <  0))) {
    new_state = state_cw_Ramp_falling;
    new_state_enter = (enter_func)cw_Ramp_enter_falling;
  }
  else
  {
    new_state = state_cw_Ramp_stopped;
    new_state_enter = (enter_func)cw_Ramp_enter_stopped;
  }
  if (new_state != m->machine.state)
    changeMachineState(cw_Ramp_To_MachineBase(m), new_state, new_state_enter);
  return 1;
}
