
#include "base_includes.h"
#include "rtio.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "cw_ANALOGOUTPUT.h"

//static const char* TAG = "ANALOGOUTPUT";
#define DEBUG_LOG 0
void cw_ANALOGOUTPUT_describe(struct cw_ANALOGOUTPUT *m);
int cw_ANALOGOUTPUT_check_state(struct cw_ANALOGOUTPUT *m);
int *cw_ANALOGOUTPUT_lookup(struct cw_ANALOGOUTPUT *m, int symbol) {
	if (symbol == sym_VALUE) return &m->VALUE;
	return 0;
}
MachineBase *cw_ANALOGOUTPUT_lookup_machine(MachineBase *m, int symbol) {
	return 0;
}
struct cw_ANALOGOUTPUT *create_cw_ANALOGOUTPUT(const char *name, int pin, MachineBase *module, int offset, int channel)
{
  struct cw_ANALOGOUTPUT *p = (struct cw_ANALOGOUTPUT *)malloc(sizeof(struct cw_ANALOGOUTPUT));
  Init_cw_ANALOGOUTPUT(p, name, pin, module, offset, channel);
  return p;
}
static void set_value(struct cw_ANALOGOUTPUT *m, const char *name, int *p, int v) {
  *p = v;
  cw_ANALOGOUTPUT_set_value(m, name, v);
}
void Init_cw_ANALOGOUTPUT(struct cw_ANALOGOUTPUT *m, const char *name, int pin, MachineBase *module, int offset, int channel)
{
  initMachineBase(&m->machine, name);
  init_io_address(&m->addr, 0, 0, 0, 16, iot_pwm, IO_STABLE);
  m->_module = module;
  m->_offset = offset;
  m->channel = channel;
  m->machine.lookup = (lookup_func)cw_ANALOGOUTPUT_lookup;
  m->machine.lookup_machine = (lookup_machine_func)cw_ANALOGOUTPUT_lookup_machine;
  m->machine.set_value = set_value;
  m->machine.describe = cw_ANALOGOUTPUT_describe;
  m->machine.state = 0;
  m->machine.check_state = ( int(*)(MachineBase*) )cw_ANALOGOUTPUT_check_state;

  setup_pwm_gpio(channel, pin, 0);
 #if 0
  ledc_channel_config_t channel_conf;
  memset(&channel_conf, 0, sizeof(ledc_channel_config_t));
  channel_conf.gpio_num = pin;
  channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
  channel_conf.duty = 0;
  channel_conf.timer_sel = LEDC_TIMER_0;
  channel_conf.hpoint = 1000;
  channel_conf.intr_type = LEDC_INTR_DISABLE;
  ledc_channel_config(&channel_conf);
  setup_pwm_gpio(channel, pin, 0);
#endif

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
void cw_ANALOGOUTPUT_set_value (struct cw_ANALOGOUTPUT *output, const char *name, uint16_t value) {
    rt_set_io_uint16(&output->addr, value);
    output->addr.status = IO_PENDING;
    publish_MQTT_property(0, &output->machine, name, value);
}
int cw_ANALOGOUTPUT_get_channel(struct cw_ANALOGOUTPUT *output) {
  return output->channel;
}
void cw_ANALOGOUTPUT_describe(struct cw_ANALOGOUTPUT *m) {
	char buf[100];
	snprintf(buf, 100, "%s: %d Class: ANALOGOUTPUT", m->machine.name, m->VALUE);
	sendMQTT(0, "/response", buf);
}
