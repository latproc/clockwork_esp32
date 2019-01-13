#include <iointerface.h>
#include "driver/gpio.h"

#define cw_ESP32_ADC1_CH0                  5
#define cw_ESP32_ADC1_CH1                  37
#define cw_ESP32_ADC1_CH2                  38
#define cw_ESP32_ADC1_CH3                  39
#define cw_ESP32_ADC1_CH4                  32
#define cw_ESP32_ADC1_CH5                  33
#define cw_ESP32_ADC1_CH6                  34
#define cw_ESP32_ADC1_CH7                  35
#define cw_ESP32_ADC2_CH0                  4
#define cw_ESP32_ADC2_CH1                  0
#define cw_ESP32_ADC2_CH2                  2
#define cw_ESP32_ADC2_CH3                  15
#define cw_ESP32_ADC2_CH4                  13
#define cw_ESP32_ADC2_CH5                  12
#define cw_ESP32_ADC2_CH6                  14
#define cw_ESP32_ADC2_CH7                  27
#define cw_ESP32_ADC2_CH8                  25
#define cw_ESP32_ADC2_CH9                  26
#define cw_ESP32_CLK_OUT1                  0
#define cw_ESP32_CLK_OUT2                  3
#define cw_ESP32_CLK_OUT3                  1
#define cw_ESP32_DAC_1                     25
#define cw_ESP32_DAC_2                     26
#define cw_ESP32_EMAC_CLK_OUT              16
#define cw_ESP32_EMAC_CLK_OUT_180          17
#define cw_ESP32_EMAC_RXD0                 25
#define cw_ESP32_EMAC_RXD1                 26
#define cw_ESP32_EMAC_RXD2                 1
#define cw_ESP32_EMAC_RXD3                 15
#define cw_ESP32_EMAC_RX_CLK               5
#define cw_ESP32_EMAC_RX_DV                27
#define cw_ESP32_EMAC_RX_ER                13
#define cw_ESP32_EMAC_TXD0                 19
#define cw_ESP32_EMAC_TXD1                 22
#define cw_ESP32_EMAC_TXD2                 14
#define cw_ESP32_EMAC_TXD3                 12
#define cw_ESP32_EMAC_TX_CLK               0
#define cw_ESP32_EMAC_TX_EN                21
#define cw_ESP32_EMAC_TX_ER                4
#define cw_ESP32_GPIO0                     0
#define cw_ESP32_GPIO1                     1
#define cw_ESP32_GPIO10                    10
#define cw_ESP32_GPIO11                    11
#define cw_ESP32_GPIO12                    12
#define cw_ESP32_GPIO13                    13
#define cw_ESP32_GPIO14                    14
#define cw_ESP32_GPIO15                    15
#define cw_ESP32_GPIO16                    16
#define cw_ESP32_GPIO17                    17
#define cw_ESP32_GPIO18                    18
#define cw_ESP32_GPIO19                    19
#define cw_ESP32_GPIO2                     2
#define cw_ESP32_GPIO21                    21
#define cw_ESP32_GPIO22                    22
#define cw_ESP32_GPIO23                    23
#define cw_ESP32_GPIO25                    25
#define cw_ESP32_GPIO26                    26
#define cw_ESP32_GPIO27                    27
#define cw_ESP32_GPIO3                     3
#define cw_ESP32_GPIO32                    32
#define cw_ESP32_GPIO33                    33
#define cw_ESP32_GPIO34                    34
#define cw_ESP32_GPIO35                    35
#define cw_ESP32_GPIO36                    5
#define cw_ESP32_GPIO37                    37
#define cw_ESP32_GPIO38                    38
#define cw_ESP32_GPIO39                    39
#define cw_ESP32_GPIO4                     4
#define cw_ESP32_GPIO5                     5
#define cw_ESP32_GPIO6                     6
#define cw_ESP32_GPIO7                     7
#define cw_ESP32_GPIO8                     8
#define cw_ESP32_GPIO9                     9
#define cw_ESP32_HS1_CLK                   6
#define cw_ESP32_HS1_CMD                   11
#define cw_ESP32_HS1_DATA0                 7
#define cw_ESP32_HS1_DATA1                 8
#define cw_ESP32_HS1_DATA2                 9
#define cw_ESP32_HS1_DATA3                 10
#define cw_ESP32_HS1_DATA4                 16
#define cw_ESP32_HS1_DATA5                 17
#define cw_ESP32_HS1_DATA6                 5
#define cw_ESP32_HS1_DATA7                 18
#define cw_ESP32_HS1_STROBE                23
#define cw_ESP32_HS2_CLK                   14
#define cw_ESP32_HS2_CMD                   15
#define cw_ESP32_HS2_DATA0                 2
#define cw_ESP32_HS2_DATA1                 4
#define cw_ESP32_HS2_DATA2                 12
#define cw_ESP32_HS2_DATA3                 13
#define cw_ESP32_HSPICLK                   14
#define cw_ESP32_HSPICS0                   15
#define cw_ESP32_HSPID                     13
#define cw_ESP32_HSPIHD                    4
#define cw_ESP32_HSPIQ                     12
#define cw_ESP32_HSPIWP                    2
#define cw_ESP32_MTCK                      13
#define cw_ESP32_MTDI                      12
#define cw_ESP32_MTDO                      15
#define cw_ESP32_MTMS                      14
#define cw_ESP32_RTC_GPIO0                 5
#define cw_ESP32_RTC_GPIO1                 37
#define cw_ESP32_RTC_GPIO10                4
#define cw_ESP32_RTC_GPIO11                0
#define cw_ESP32_RTC_GPIO12                2
#define cw_ESP32_RTC_GPIO13                15
#define cw_ESP32_RTC_GPIO14                13
#define cw_ESP32_RTC_GPIO15                12
#define cw_ESP32_RTC_GPIO16                14
#define cw_ESP32_RTC_GPIO17                27
#define cw_ESP32_RTC_GPIO2                 38
#define cw_ESP32_RTC_GPIO3                 39
#define cw_ESP32_RTC_GPIO4                 34
#define cw_ESP32_RTC_GPIO5                 35
#define cw_ESP32_RTC_GPIO6                 25
#define cw_ESP32_RTC_GPIO7                 26
#define cw_ESP32_RTC_GPIO8                 33
#define cw_ESP32_RTC_GPIO9                 32
#define cw_ESP32_SD2_CLK                   14
#define cw_ESP32_SD2_CMD                   15
#define cw_ESP32_SD2_DATA0                 2
#define cw_ESP32_SD2_DATA1                 4
#define cw_ESP32_SD2_DATA2                 12
#define cw_ESP32_SD2_DATA3                 13
#define cw_ESP32_SD_CLK                    6
#define cw_ESP32_SD_CMD                    11
#define cw_ESP32_SD_DATA0                  7
#define cw_ESP32_SD_DATA1                  8
#define cw_ESP32_SD_DATA2                  9
#define cw_ESP32_SD_DATA3                  10
#define cw_ESP32_SPICLK                    6
#define cw_ESP32_SPICS0                    11
#define cw_ESP32_SPID                      8
#define cw_ESP32_SPIHD                     9
#define cw_ESP32_SPIQ                      7
#define cw_ESP32_SPIWP                     10
#define cw_ESP32_TOUCH0                    4
#define cw_ESP32_TOUCH1                    0
#define cw_ESP32_TOUCH2                    2
#define cw_ESP32_TOUCH3                    15
#define cw_ESP32_TOUCH4                    13
#define cw_ESP32_TOUCH5                    12
#define cw_ESP32_TOUCH6                    14
#define cw_ESP32_TOUCH7                    27
#define cw_ESP32_TOUCH8                    33
#define cw_ESP32_TOUCH9                    32
#define cw_ESP32_U0CTS                     19
#define cw_ESP32_U0RTS                     22
#define cw_ESP32_U0RXD                     3
#define cw_ESP32_U0TXD                     1
#define cw_ESP32_U1CTS                     6
#define cw_ESP32_U1RTS                     11
#define cw_ESP32_U1RXD                     9
#define cw_ESP32_U1TXD                     10
#define cw_ESP32_U2CTSA                    8
#define cw_ESP32_U2RTS                     7
#define cw_ESP32_U2RXD                     16
#define cw_ESP32_U2TXD                     17
#define cw_ESP32_VSPICLK                   18
#define cw_ESP32_VSPICS0                   5
#define cw_ESP32_VSPID                     23
#define cw_ESP32_VSPIHD                    21
#define cw_ESP32_VSPIQ                     19
#define cw_ESP32_VSPIWP                    22
#define cw_ESP32_XN_32K                    33
#define cw_ESP32_XP_32K                    32
#define cw_OLIMEX_GATEWAY32_BUT1                      34
#define cw_OLIMEX_GATEWAY32_GPIO11                    11
#define cw_OLIMEX_GATEWAY32_GPIO16                    16
#define cw_OLIMEX_GATEWAY32_GPIO17                    17
#define cw_OLIMEX_GATEWAY32_GPIO32                    32
#define cw_OLIMEX_GATEWAY32_GPIO5                     5
#define cw_OLIMEX_GATEWAY32_LED                       33
#define cw_LOLIBOT_IO0                       0
#define cw_LOLIBOT_IO13                      13
#define cw_LOLIBOT_IO23                      23
#define cw_LOLIBOT_LEFT_FWD                  13
#define cw_LOLIBOT_LEFT_REV                  15
#define cw_LOLIBOT_RIGHT_FWD                 23
#define cw_LOLIBOT_RIGHT_REV                 5
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
struct PointOutput *cw_inst_left_dir = create_cw_PointOutput("left_dir", cw_OLIMEX_GATEWAY32_GPIO32, 0);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_GPIO32);
gpio_set_direction(cw_OLIMEX_GATEWAY32_GPIO32, GPIO_MODE_OUTPUT);
gpio_set_level(cw_OLIMEX_GATEWAY32_GPIO32, 0);
{
	struct MachineBase *m = cw_PointOutput_To_MachineBase(cw_inst_left_dir);
	if (m->init) m->init();
	struct IOItem *item_cw_inst_left_dir = IOItem_create(m, cw_PointOutput_getAddress(cw_inst_left_dir), cw_OLIMEX_GATEWAY32_GPIO32);
	RTIOInterface_add(interface, item_cw_inst_left_dir);
}
struct PointOutput *cw_inst_right_dir = create_cw_PointOutput("right_dir", cw_OLIMEX_GATEWAY32_LED, 0);
gpio_pad_select_gpio(cw_OLIMEX_GATEWAY32_LED);
gpio_set_direction(cw_OLIMEX_GATEWAY32_LED, GPIO_MODE_OUTPUT);
gpio_set_level(cw_OLIMEX_GATEWAY32_LED, 0);
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
