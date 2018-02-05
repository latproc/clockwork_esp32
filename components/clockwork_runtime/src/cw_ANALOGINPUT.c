
#include "base_includes.h"
#include "cw_ANALOGINPUT.h"
//static const char* TAG = "ANALOGINPUT";
#define DEBUG_LOG 0
struct cw_ANALOGINPUT
{
  MachineBase machine;
  int gpio_pin;
  struct IOAddress addr;
  MachineBase *_module;
  int _offset;
  MachineBase *_filter_settings;
  int _adc_channel;
};
int cw_ANALOGINPUT_check_state(struct cw_ANALOGINPUT *m);
struct cw_ANALOGINPUT *create_cw_ANALOGINPUT(const char *name, int pin, MachineBase *module, int offset, int channel, MachineBase *filter_settings)
{
  struct cw_ANALOGINPUT *p = (struct cw_ANALOGINPUT *)malloc(sizeof(struct cw_ANALOGINPUT));
  Init_cw_ANALOGINPUT(p, name, pin, module, offset, channel, filter_settings);
  return p;
}
void Init_cw_ANALOGINPUT(struct cw_ANALOGINPUT *m, const char *name, int pin, MachineBase *module, int offset, int channel, MachineBase *filter_settings)
{
  initMachineBase(&m->machine, name);
  init_io_address(&m->addr, 0, 0, 0,  16, iot_none, IO_STABLE);
  m->_module = module;
  m->_offset = offset;
  m->_adc_channel = channel;
  m->_filter_settings = filter_settings;
  m->machine.state = 0;
  m->machine.check_state = ( int(*)(MachineBase*) )cw_ANALOGINPUT_check_state;
  markPending(&m->machine);
}
struct IOAddress *cw_ANALOGINPUT_getAddress(struct cw_ANALOGINPUT *p)
{
  return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
int cw_ANALOGINPUT_getChannel(struct cw_ANALOGINPUT *p) {
  return p->_adc_channel;
}
MachineBase *cw_ANALOGINPUT_To_MachineBase(struct cw_ANALOGINPUT *p)
{
  return &p->machine;
}
int ANALOGINPUT_enter_INIT(struct cw_ANALOGINPUT *m, ccrContParam)
{
  m->machine.execute = 0;
  return 1;
}
int ANALOGINPUT_enter_stable(struct cw_ANALOGINPUT *m, ccrContParam)
{
  m->machine.execute = 0;
  return 1;
}
int ANALOGINPUT_enter_unstable(struct cw_ANALOGINPUT *m, ccrContParam)
{
  m->machine.execute = 0;
  return 1;
}
int cw_ANALOGINPUT_check_state(struct cw_ANALOGINPUT *m)
{
  int new_state = 0;
  enter_func new_state_enter = 0;
  if (new_state != m->machine.state)
    changeMachineState(cw_ANALOGINPUT_To_MachineBase(m), new_state, new_state_enter);
  return 1;
}
