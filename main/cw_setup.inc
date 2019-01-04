#include <iointerface.h>
#include "driver/gpio.h"

#define cw_OLIMEX_GATEWAY32_LED 33
#define cw_OLIMEX_GATEWAY32_BUT1 34
#define cw_OLIMEX_GATEWAY32_GPIO16 16
#define cw_OLIMEX_GATEWAY32_GPIO17 17
#define cw_OLIMEX_GATEWAY32_GPIO32 32
#define cw_OLIMEX_GATEWAY32_GPIO11 11
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
struct cw_ANALOGOUTPUT *cw_inst_left = create_cw_ANALOGOUTPUT("left", cw_OLIMEX_GATEWAY32_GPIO16, 0, 0, LEDC_CHANNEL_0);
{
	struct MachineBase *m = cw_ANALOGOUTPUT_To_MachineBase(cw_inst_left);
	cw_inst_left->VALUE = 0;
	if (m->init) m->init();
	struct IOItem *item_cw_inst_left = IOItem_create(m, cw_ANALOGOUTPUT_getAddress(cw_inst_left), cw_OLIMEX_GATEWAY32_GPIO16);
	RTIOInterface_add(interface, item_cw_inst_left);
}
struct cw_ANALOGOUTPUT *cw_inst_right = create_cw_ANALOGOUTPUT("right", cw_OLIMEX_GATEWAY32_GPIO17, 0, 0, LEDC_CHANNEL_0);
{
	struct MachineBase *m = cw_ANALOGOUTPUT_To_MachineBase(cw_inst_right);
	cw_inst_right->VALUE = 0;
	if (m->init) m->init();
	struct IOItem *item_cw_inst_right = IOItem_create(m, cw_ANALOGOUTPUT_getAddress(cw_inst_right), cw_OLIMEX_GATEWAY32_GPIO17);
	RTIOInterface_add(interface, item_cw_inst_right);
}
struct PointOutput *cw_inst_left_dir = create_cw_PointOutput("left_dir", cw_OLIMEX_GATEWAY32_GPIO32);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_GPIO32);
gpio_set_direction(cw_OLIMEX_GATEWAY32_GPIO32, GPIO_MODE_OUTPUT);
{
	struct MachineBase *m = cw_PointOutput_To_MachineBase(cw_inst_left_dir);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_left_dir = IOItem_create(m, cw_PointOutput_getAddress(cw_inst_left_dir), cw_OLIMEX_GATEWAY32_GPIO32);
	RTIOInterface_add(interface, item_cw_inst_left_dir);
}
struct PointOutput *cw_inst_right_dir = create_cw_PointOutput("right_dir", cw_OLIMEX_GATEWAY32_LED);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_LED);
gpio_set_direction(cw_OLIMEX_GATEWAY32_LED, GPIO_MODE_OUTPUT);
{
	struct MachineBase *m = cw_PointOutput_To_MachineBase(cw_inst_right_dir);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_right_dir = IOItem_create(m, cw_PointOutput_getAddress(cw_inst_right_dir), cw_OLIMEX_GATEWAY32_LED);
	RTIOInterface_add(interface, item_cw_inst_right_dir);
}
struct cw_Pulse *cw_inst_pulser = create_cw_Pulse("pulser");
{
	struct MachineBase *m = cw_Pulse_To_MachineBase(cw_inst_pulser);
	cw_inst_pulser->delay = 50;
	if (m->init) m->init();
}
struct cw_Ramp *cw_inst_left_motor = create_cw_Ramp("left_motor", (MachineBase*)cw_inst_pulser, (MachineBase*)cw_inst_left, (MachineBase*)cw_inst_left_dir);
{
	struct MachineBase *m = cw_Ramp_To_MachineBase(cw_inst_left_motor);
	cw_inst_left_motor->direction = 0;
	cw_inst_left_motor->end = 15000;
	cw_inst_left_motor->min = 5000;
	cw_inst_left_motor->start = 13000;
	cw_inst_left_motor->step = 800;
	if (m->init) m->init();
}
struct cw_Ramp *cw_inst_right_motor = create_cw_Ramp("right_motor", (MachineBase*)cw_inst_pulser, (MachineBase*)cw_inst_right, (MachineBase*)cw_inst_right_dir);
{
	struct MachineBase *m = cw_Ramp_To_MachineBase(cw_inst_right_motor);
	cw_inst_right_motor->direction = 0;
	cw_inst_right_motor->end = 30000;
	cw_inst_right_motor->min = 5000;
	cw_inst_right_motor->start = 5000;
	cw_inst_right_motor->step = 800;
	if (m->init) m->init();
}
struct cw_DebouncedInput *cw_inst_d_button = create_cw_DebouncedInput("d_button", (MachineBase*)cw_inst_button);
{
	struct MachineBase *m = cw_DebouncedInput_To_MachineBase(cw_inst_d_button);
	cw_inst_d_button->debounce_time = 100;
	cw_inst_d_button->off_time = 50;
	if (m->init) m->init();
}
struct cw_SpeedSelect *cw_inst_speed_select = create_cw_SpeedSelect("speed_select", (MachineBase*)cw_inst_d_button, (MachineBase*)cw_inst_pulser);
{
	struct MachineBase *m = cw_SpeedSelect_To_MachineBase(cw_inst_speed_select);
	if (m->init) m->init();
}
