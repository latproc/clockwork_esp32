#include "driver/gpio.h"
#include "driver/adc.h"

#include "base_includes.h"
#include "cw_ANALOGINPUT.h"
//static const char* TAG = "ANALOGINPUT";
#define DEBUG_LOG 0
void cw_ANALOGINPUT_describe(struct cw_ANALOGINPUT *m);
int cw_ANALOGINPUT_check_state(struct cw_ANALOGINPUT *m);
int *cw_ANALOGINPUT_lookup(struct cw_ANALOGINPUT *m, int symbol) {
  if (symbol == sym_VALUE) return &m->VALUE;
  return 0;
}
MachineBase *cw_ANALOGINPUT_lookup_machine(MachineBase *m, int symbol) {
  return 0;
}
struct cw_ANALOGINPUT *create_cw_ANALOGINPUT(const char *name, int pin, MachineBase *module, int offset, int channel, MachineBase *filter_settings)
{
  struct cw_ANALOGINPUT *p = (struct cw_ANALOGINPUT *)malloc(sizeof(struct cw_ANALOGINPUT));
  Init_cw_ANALOGINPUT(p, name, pin, module, offset, channel, filter_settings);
  return p;
}
void cw_ANALOGINPUT_set_value (struct cw_ANALOGINPUT *input, const char *name, int *p, int v) {
    *p = v;
    input->IOTIME = upTime();
    // do not publish analogue input changes (noisy)
    //publish_MQTT_property(0, &input->machine, name, v);
}
void Init_cw_ANALOGINPUT(struct cw_ANALOGINPUT *m, const char *name, int pin, MachineBase *module, int offset, int channel, MachineBase *filter_settings)
{
  initMachineBase(&m->machine, name);
  m->machine.class_name = "ANALOGINPUT";
  init_io_address(&m->addr, 0, 0, 0,  16, iot_adc, IO_STABLE);
  m->gpio_pin = pin;
  m->_module = module;
  m->_offset = offset;
  m->_adc_channel = channel;
  m->Position = 0;
  m->IOTIME = 0;
  m->Velocity = 0;
  m->_filter_settings = filter_settings;
  m->machine.lookup = (lookup_func)cw_ANALOGINPUT_lookup;
  m->machine.lookup_machine = (lookup_machine_func)cw_ANALOGINPUT_lookup_machine;
  m->machine.set_value = (set_value_func)cw_ANALOGINPUT_set_value;
  m->machine.state = 0;
  m->machine.check_state = ( int(*)(MachineBase*) )cw_ANALOGINPUT_check_state;
  m->machine.describe = (describe_func)cw_ANALOGINPUT_describe;
  adc1_config_channel_atten(channel, ADC_ATTEN_DB_0);
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
void cw_ANALOGINPUT_describe(struct cw_ANALOGINPUT *m) {
	char buf[100];
	snprintf(buf, 100, "%s: %d Class: ANALOGINPUT", m->machine.name, m->VALUE);
	sendMQTT(0, "/response", buf);
}
