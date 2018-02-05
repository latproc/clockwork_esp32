
#include "base_includes.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "cw_ANALOGOUTPUT.h"

//static const char* TAG = "ANALOGOUTPUT";
#define DEBUG_LOG 0
struct cw_ANALOGOUTPUT
{
  MachineBase machine;
  int gpio_pin;
  struct IOAddress addr;
  MachineBase *_module;
  int _offset;
  int _pwm_channel; // ESP32 specific - the channel that the gpio is linked to
};
int cw_ANALOGOUTPUT_check_state(struct cw_ANALOGOUTPUT *m);
struct cw_ANALOGOUTPUT *create_cw_ANALOGOUTPUT(const char *name, int pin, MachineBase *module, int offset, int channel)
{
  struct cw_ANALOGOUTPUT *p = (struct cw_ANALOGOUTPUT *)malloc(sizeof(struct cw_ANALOGOUTPUT));
  Init_cw_ANALOGOUTPUT(p, name, pin, module, offset, channel);
  return p;
}
void Init_cw_ANALOGOUTPUT(struct cw_ANALOGOUTPUT *m, const char *name, int pin, MachineBase *module, int offset, int channel)
{
  initMachineBase(&m->machine, name);
  init_io_address(&m->addr, 0, 0, 0, 16, iot_pwm, IO_STABLE);
  m->_module = module;
  m->_offset = offset;
  m->_pwm_channel = channel;
  setup_pwm_gpio(channel, pin, 0);
  
  m->machine.state = 0;
  m->machine.check_state = ( int(*)(MachineBase*) )cw_ANALOGOUTPUT_check_state;
  markPending(&m->machine);
}
struct IOAddress *cw_ANALOGOUTPUT_getAddress(struct cw_ANALOGOUTPUT *p)
{
  return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_ANALOGOUTPUT_To_MachineBase(struct cw_ANALOGOUTPUT *p)
{
  return &p->machine;
}
int ANALOGOUTPUT_enter_INIT(struct cw_ANALOGOUTPUT *m, ccrContParam)
{
  m->machine.execute = 0;
  return 1;
}
int ANALOGOUTPUT_enter_stable(struct cw_ANALOGOUTPUT *m, ccrContParam)
{
  m->machine.execute = 0;
  return 1;
}
int ANALOGOUTPUT_enter_unstable(struct cw_ANALOGOUTPUT *m, ccrContParam)
{
  m->machine.execute = 0;
  return 1;
}
int cw_ANALOGOUTPUT_check_state(struct cw_ANALOGOUTPUT *m)
{
  int new_state = 0;
  enter_func new_state_enter = 0;
  if (new_state != m->machine.state)
    changeMachineState(cw_ANALOGOUTPUT_To_MachineBase(m), new_state, new_state_enter);
  return 1;
}
void cw_ANALOGOUTPUT_set_value (struct cw_ANALOGOUTPUT *output, uint16_t value) {
    rt_set_io_uint16(&output->addr, value);
    output->addr.status = IO_PENDING;
}
int cw_ANALOGOUTPUT_get_channel(struct cw_ANALOGOUTPUT *output) {
  return output->_pwm_channel;
}

