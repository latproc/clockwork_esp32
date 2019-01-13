
#include "base_includes.h"
#include "esp32_digital_led_lib.h"

#include "cw_DIGITALLEDS.h"
#define DEBUG_LOG 0
//static const char* TAG = "DIGITALLEDS";

struct StrandListItem {
	struct StrandListItem *next;
	strand_t *strand;
};
static struct StrandListItem *led_strands = 0;

void push_strand(strand_t *src) {
	strand_t *s = (strand_t*)malloc(sizeof(strand_t));
	memcpy(s, src, sizeof(strand_t));

	struct StrandListItem *sli = (struct StrandListItem*)malloc(sizeof(struct StrandListItem));
	sli->next = led_strands;
	sli->strand = s;
	led_strands = sli;
}

void cwDigitalLedsTask(void *pvParameter) {
    while(1) {
        vTaskDelay(10);
        struct StrandListItem *sli = led_strands;
		while (sli) {
        	//digitalLeds_resetPixels(sli->strand);
			sli = sli->next;
		}
    }
}

#define state_cw_INIT 1
struct cw_DIGITALLEDS_Vars {
	struct cw_DIGITALLEDS *m;
	unsigned int l_INIT;
	Value *l_channel;
	Value *l_max_output;
	Value *l_num_pixels;
	unsigned int *l_out;
	Value *l_pin;
};
struct cw_DIGITALLEDS_Vars_backup {
	struct cw_DIGITALLEDS  m;
	unsigned int l_INIT;
	Value  l_channel;
	Value  l_max_output;
	Value  l_num_pixels;
	unsigned int  l_out;
	Value  l_pin;
};
static void init_Vars(struct cw_DIGITALLEDS *m, struct cw_DIGITALLEDS_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_channel = &m->channel;
	v->l_max_output = &m->max_output;
	v->l_num_pixels = &m->num_pixels;
	v->l_out = &m->_out->state;
	v->l_pin = &m->pin;
}
static void backup_Vars(struct cw_DIGITALLEDS *m) {
	struct cw_DIGITALLEDS_Vars *v = m->vars;
	struct cw_DIGITALLEDS_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_channel = *v->l_channel;
	b->l_max_output = *v->l_max_output;
	b->l_num_pixels = *v->l_num_pixels;
	b->l_out = *v->l_out;
	b->l_pin = *v->l_pin;
}
Value *cw_DIGITALLEDS_lookup(struct cw_DIGITALLEDS *m, int symbol) {
	if (symbol == sym_channel) return &m->channel;
	if (symbol == sym_max_output) return &m->max_output;
	if (symbol == sym_num_pixels) return &m->num_pixels;
	if (symbol == sym_pin) return &m->pin;
	return 0;
}
MachineBase *cw_DIGITALLEDS_lookup_machine(struct cw_DIGITALLEDS *m, int symbol) {
	if (symbol == sym_out) return m->_out;
	return 0;
}
void cw_DIGITALLEDS_describe(struct cw_DIGITALLEDS *m);
int cw_DIGITALLEDS_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_DIGITALLEDS_check_state(struct cw_DIGITALLEDS *m);
struct cw_DIGITALLEDS *create_cw_DIGITALLEDS(const char *name, MachineBase *out) {
	struct cw_DIGITALLEDS *p = (struct cw_DIGITALLEDS *)malloc(sizeof(struct cw_DIGITALLEDS));
	Init_cw_DIGITALLEDS(p, name, out);
	return p;
}
int cw_DIGITALLEDS_INIT_enter(struct cw_DIGITALLEDS *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_DIGITALLEDS_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	//struct cw_DIGITALLEDS *m = (struct cw_DIGITALLEDS *)obj;
	markPending(obj);
	return 1;
}

void Init_cw_DIGITALLEDS(struct cw_DIGITALLEDS *m, const char *name, MachineBase *out) {
	initMachineBase(&m->machine, name);
	m->machine.class_name = "DIGITALLEDS";
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_out = out;
	if (out) MachineDependencies_add(out, cw_DIGITALLEDS_To_MachineBase(m));
	m->channel = 1;
	m->max_output = 32;
	m->num_pixels = 8;
	m->pin = *out->lookup(out, sym_pin); // TODO: need validation
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_DIGITALLEDS_check_state;
	m->machine.handle = (message_func)cw_DIGITALLEDS_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_DIGITALLEDS_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_DIGITALLEDS_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_DIGITALLEDS_describe;
	m->vars = (struct cw_DIGITALLEDS_Vars *)malloc(sizeof(struct cw_DIGITALLEDS_Vars));
	m->backup = (struct cw_DIGITALLEDS_Vars_backup *)malloc(sizeof(struct cw_DIGITALLEDS_Vars_backup));
	init_Vars(m, m->vars);
	int started = led_strands != 0; // to decide whether to start the led task or not
	{
		strand_t strand = { // Avoid using any of the strapping pins on the ESP32
  		  .rmtChannel = m->channel, 
		  .gpioNum = 19, 
		  .ledType = LED_SK6812W_V1, 
		  .brightLimit = 64, 
		  .numPixels =  8, 
		  .pixels = 0, 
		  ._stateVars = 0
		};
		push_strand(&strand);
	}
	digitalLeds_initStrands(led_strands->strand, 1);
	if (!started)
		xTaskCreate(&cwDigitalLedsTask, "cwDigitalLedsTask", 10000, NULL, 3, NULL);
	strand_t *s = led_strands->strand;
	for (uint16_t i = 0; i < s->numPixels; i++) {
      //pStrand->pixels[i] = pixelFromRGB(color1.r, color1.g, color1.b);
      if (i%4 == 0) s->pixels[i] = pixelFromRGBW(12, 0, 0, 0);
      if (i%4 == 1) s->pixels[i] = pixelFromRGBW(0, 12, 0, 0);
      if (i%4 == 2) s->pixels[i] = pixelFromRGBW(0, 0, 16, 0);
      if (i%4 == 3) s->pixels[i] = pixelFromRGBW(0, 0, 0, 8);
	}
	digitalLeds_updatePixels(s);
	MachineActions_add(cw_DIGITALLEDS_To_MachineBase(m), (enter_func)cw_DIGITALLEDS_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_DIGITALLEDS_getAddress(struct cw_DIGITALLEDS *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_DIGITALLEDS_To_MachineBase(struct cw_DIGITALLEDS *p) { return &p->machine; }

int cw_DIGITALLEDS_check_state(struct cw_DIGITALLEDS *m) {
	//struct cw_DIGITALLEDS_Vars_backup *v = m->backup;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_DIGITALLEDS_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
void cw_DIGITALLEDS_describe(struct cw_DIGITALLEDS *m) {
	//struct cw_DIGITALLEDS_Vars_backup *v = m->backup;
	char buf[100];
	snprintf(buf, 100, "%s: %s  Class: DIGITALLEDS", m->machine.name, name_from_id(m->machine.state));
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT(0,"/response", buf);
}
