
#include "base_includes.h"
#include "esp32_digital_led_lib.h"

#include "cw_LEDSTRIP.h"
#include "cw_DIGITALLED.h"
#define DEBUG_LOG 0
//static const char* TAG = "DIGITALLEDS";

struct private_strand {
	strand_t s;
};
struct StrandListItem {
	struct StrandListItem *next;
	struct cw_LEDSTRIP *strip;
};
static struct StrandListItem *led_strands = 0;

void push_strip(struct cw_LEDSTRIP *strip) {
	struct StrandListItem *sli = (struct StrandListItem*)malloc(sizeof(struct StrandListItem));
	sli->next = led_strands;
	sli->strip = strip;
	led_strands = sli;
}

struct DigitalLedItem {
    struct list_node list;
	struct cw_DIGITALLED *led;
};

void cwDigitalLedsTask(void *pvParameter) {
    while(1) {
        vTaskDelay(20);
        struct StrandListItem *sli = led_strands;
		while (sli) {
			struct cw_LEDSTRIP *strip = sli->strip;
			int num_pix = strip->strand->s.numPixels;
		    struct DigitalLedItem *item = 0;
			digitalLeds_resetPixels(&strip->strand->s);
			list_for_each(&strip->leds, item, list) {
				struct cw_DIGITALLED *led = item->led;
				if (led->position >= 0 && led->position < num_pix) {
					strip->strand->s.pixels[led->position] = pixelFromRGBW((uint8_t)led->r, (uint8_t)led->g, (uint8_t)led->b, 0);
				}
			}
			digitalLeds_updatePixels(&strip->strand->s);
			sli = sli->next;
		}
    }
}

#define state_cw_INIT 1
struct cw_LEDSTRIP_Vars {
	struct cw_LEDSTRIP *m;
	unsigned int l_INIT;
	Value *l_channel;
	Value *l_max_output;
	Value *l_num_pixels;
	unsigned int *l_out;
	Value *l_pin;
};
struct cw_LEDSTRIP_Vars_backup {
	struct cw_LEDSTRIP  m;
	unsigned int l_INIT;
	Value  l_channel;
	Value  l_max_output;
	Value  l_num_pixels;
	unsigned int  l_out;
	Value  l_pin;
};
static void init_Vars(struct cw_LEDSTRIP *m, struct cw_LEDSTRIP_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_channel = &m->channel;
	v->l_max_output = &m->max_output;
	v->l_num_pixels = &m->num_pixels;
	v->l_out = &m->_out->state;
	v->l_pin = &m->pin;
}
static void backup_Vars(struct cw_LEDSTRIP *m) {
	struct cw_LEDSTRIP_Vars *v = m->vars;
	struct cw_LEDSTRIP_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_channel = *v->l_channel;
	b->l_max_output = *v->l_max_output;
	b->l_num_pixels = *v->l_num_pixels;
	b->l_out = *v->l_out;
	b->l_pin = *v->l_pin;
}
Value *cw_LEDSTRIP_lookup(struct cw_LEDSTRIP *m, int symbol) {
	if (symbol == sym_channel) return &m->channel;
	if (symbol == sym_max_output) return &m->max_output;
	if (symbol == sym_num_pixels) return &m->num_pixels;
	if (symbol == sym_pin) return &m->pin;
	return 0;
}
MachineBase *cw_LEDSTRIP_lookup_machine(struct cw_LEDSTRIP *m, int symbol) {
	if (symbol == sym_out) return m->_out;
	return 0;
}
void cw_LEDSTRIP_describe(struct cw_LEDSTRIP *m);
int cw_LEDSTRIP_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_LEDSTRIP_check_state(struct cw_LEDSTRIP *m);
struct cw_LEDSTRIP *create_cw_LEDSTRIP(const char *name, MachineBase *out, MachineBase *led_type) {
	struct cw_LEDSTRIP *p = (struct cw_LEDSTRIP *)malloc(sizeof(struct cw_LEDSTRIP));
	Init_cw_LEDSTRIP(p, name, out, led_type);
	return p;
}
int cw_LEDSTRIP_INIT_enter(struct cw_LEDSTRIP *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_LEDSTRIP_handle_message(struct MachineBase *obj, struct MachineBase *source, int state) {
	//struct cw_LEDSTRIP *m = (struct cw_LEDSTRIP *)obj;
	markPending(obj);
	return 1;
}
void add_led_to_strip(struct cw_LEDSTRIP *strip, struct cw_DIGITALLED *led) {
    struct DigitalLedItem *item = 0, *next = 0;
	list_for_each_safe(&strip->leds, item, next, list) {
		if (item->led->position > led->position)
			break;
	}
	if (item) {
		struct DigitalLedItem *led_item = (struct DigitalLedItem*) malloc(sizeof(struct DigitalLedItem));
		led_item->led = led;
		list_add_before(&strip->leds, &item->list, &led_item->list);
	}
}

void Init_cw_LEDSTRIP(struct cw_LEDSTRIP *m, const char *name, MachineBase *out, MachineBase *led_type) {
	initMachineBase(&m->machine, name);
	m->machine.class_name = "DIGITALLEDS";
	//init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_out = out;
	m->_led_type = led_type;
	if (out) MachineDependencies_add(out, cw_LEDSTRIP_To_MachineBase(m));
	m->channel = 1;
	m->max_output = 32;
	m->num_pixels = 8;
	m->pin = *out->lookup(out, sym_pin); // TODO: need validation
	list_head_init(&m->leds);
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_LEDSTRIP_check_state;
	m->machine.handle = (message_func)cw_LEDSTRIP_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_LEDSTRIP_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_LEDSTRIP_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_LEDSTRIP_describe;
	m->vars = (struct cw_LEDSTRIP_Vars *)malloc(sizeof(struct cw_LEDSTRIP_Vars));
	m->backup = (struct cw_LEDSTRIP_Vars_backup *)malloc(sizeof(struct cw_LEDSTRIP_Vars_backup));
	init_Vars(m, m->vars);
	backup_Vars(m);
	int started = led_strands != 0; // to decide whether to start the led task or not
	{
		strand_t strand = { // Avoid using any of the strapping pins on the ESP32
  		  .rmtChannel = m->channel, 
		  .gpioNum = m->pin, 
		  .ledType = -1, //use the custom settings 
		  .brightLimit = m->max_output, 
		  .numPixels =  m->num_pixels, 
		  .pixels = 0, 
		  ._stateVars = 0,
		  .customLedType = {
			.bytesPerPixel = *led_type->lookup(led_type, sym_bytesPerPixel),
			.T0H = *led_type->lookup(led_type, sym_T0H),
			.T1H = *led_type->lookup(led_type, sym_T1H),
			.T0L = *led_type->lookup(led_type, sym_T0L),
			.T1L = *led_type->lookup(led_type, sym_T1L),
			.TRS = *led_type->lookup(led_type, sym_TRS)
		  }
		};
		m->strand = (struct private_strand*)malloc(sizeof(struct private_strand));
		memcpy(&m->strand->s, &strand, sizeof(strand_t));
	}
	digitalLeds_initStrands(&m->strand->s, 1);
	strand_t *s = &m->strand->s;
	for (uint16_t i = 0; i < s->numPixels; i++) {
    	s->pixels[i] = pixelFromRGBW(0, 0, 0, 0);
	}
	digitalLeds_updatePixels(s);
	push_strip(m);
	if (!started)
		xTaskCreate(&cwDigitalLedsTask, "cwDigitalLedsTask", 10000, NULL, 3, NULL);
	MachineActions_add(cw_LEDSTRIP_To_MachineBase(m), (enter_func)cw_LEDSTRIP_INIT_enter);
	markPending(&m->machine);
}
//struct IOAddress *cw_LEDSTRIP_getAddress(struct cw_LEDSTRIP *p) {
//	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
//}
MachineBase *cw_LEDSTRIP_To_MachineBase(struct cw_LEDSTRIP *p) { return &p->machine; }

int cw_LEDSTRIP_check_state(struct cw_LEDSTRIP *m) {
	//struct cw_LEDSTRIP_Vars_backup *v = m->backup;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_LEDSTRIP_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
void cw_LEDSTRIP_describe(struct cw_LEDSTRIP *m) {
	//struct cw_LEDSTRIP_Vars_backup *v = m->backup;
	char buf[100];
	snprintf(buf, 100, "%s: %s  Class: DIGITALLEDS", m->machine.name, name_from_id(m->machine.state));
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT(0,"/response", buf);
}
