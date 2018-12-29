#include <iointerface.h>
#include "driver/gpio.h"

#define cw_OLIMEX_GATEWAY32_LED 33
#define cw_OLIMEX_GATEWAY32_BUT1 34
#define cw_OLIMEX_GATEWAY32_GPIO16 16
struct RTIOInterface *interface = RTIOInterface_get();
while (!interface) {
  taskYIELD();
  interface = RTIOInterface_get();
}
struct PointInput *cw_inst_button = create_cw_PointInput("button", cw_OLIMEX_GATEWAY32_BUT1);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_BUT1);
gpio_set_direction(cw_OLIMEX_GATEWAY32_BUT1, GPIO_MODE_INPUT);
{
	struct MachineBase *m = cw_PointInput_To_MachineBase(cw_inst_button);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_button = IOItem_create(m, cw_PointInput_getAddress(cw_inst_button), cw_OLIMEX_GATEWAY32_BUT1);
	RTIOInterface_add(interface, item_cw_inst_button);
}
struct PointOutput *cw_inst_led = create_cw_PointOutput("led", cw_OLIMEX_GATEWAY32_LED);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_LED);
gpio_set_direction(cw_OLIMEX_GATEWAY32_LED, GPIO_MODE_OUTPUT);
{
	struct MachineBase *m = cw_PointOutput_To_MachineBase(cw_inst_led);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_led = IOItem_create(m, cw_PointOutput_getAddress(cw_inst_led), cw_OLIMEX_GATEWAY32_LED);
	RTIOInterface_add(interface, item_cw_inst_led);
}
struct ANALOGOUTPUT *cw_inst_aout = create_cw_ANALOGOUTPUT("aout", cw_OLIMEX_GATEWAY32_GPIO16, 0, 0, LEDC_CHANNEL_0);
{
	struct MachineBase *m = cw_ANALOGOUTPUT_To_MachineBase(cw_inst_aout);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_aout = IOItem_create(m, cw_ANALOGOUTPUT_getAddress(cw_inst_aout), cw_OLIMEX_GATEWAY32_GPIO16);
	RTIOInterface_add(interface, item_cw_inst_aout);
}
struct Pulse *cw_inst_pulser = create_cw_Pulse("pulser", cw_inst_led);
{
	struct MachineBase *m = cw_Pulse_To_MachineBase(cw_inst_pulser);
	if (m->init) m->init();
}
struct Ramp *cw_inst_ramp = create_cw_Ramp("ramp", cw_inst_pulser, cw_inst_aout);
{
	struct MachineBase *m = cw_Ramp_To_MachineBase(cw_inst_ramp);
	if (m->init) m->init();
}
struct DebouncedInput *cw_inst_d_button = create_cw_DebouncedInput("d_button", cw_inst_button);
{
	struct MachineBase *m = cw_DebouncedInput_To_MachineBase(cw_inst_d_button);
	if (m->init) m->init();
}
struct SpeedSelect *cw_inst_speed_select = create_cw_SpeedSelect("speed_select", cw_inst_d_button, cw_inst_pulser);
{
	struct MachineBase *m = cw_SpeedSelect_To_MachineBase(cw_inst_speed_select);
	if (m->init) m->init();
}
