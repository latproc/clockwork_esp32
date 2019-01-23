
#include "base_includes.h"
#include "cw_DIGITALLED.h"
#define DEBUG_LOG 0
#if DEBUG_LOG
static const char* TAG = "DIGITALLED";
#endif

#define state_cw_INIT 1
struct cw_DIGITALLED_Vars {
	struct cw_DIGITALLED *m;
	unsigned int l_INIT;
	Value *l_b;
	Value *l_g;
	Value *l_max;
	Value *l_position;
	Value *l_r;
	unsigned int *l_strip;
};
struct cw_DIGITALLED_Vars_backup {
	struct cw_DIGITALLED  m;
	unsigned int l_INIT;
	Value  l_b;
	Value  l_g;
	Value  l_max;
	Value  l_position;
	Value  l_r;
	unsigned int  l_strip;
};
static void init_Vars(struct cw_DIGITALLED *m, struct cw_DIGITALLED_Vars *v) {
	v->m = m;
	v->l_INIT = state_cw_INIT;
	v->l_b = &m->b;
	v->l_g = &m->g;
	v->l_max = &m->max;
	v->l_position = &m->position;
	v->l_r = &m->r;
	v->l_strip = &m->_strip->state;
}
static void backup_Vars(struct cw_DIGITALLED *m) {
	struct cw_DIGITALLED_Vars *v = m->vars;
	struct cw_DIGITALLED_Vars_backup *b = m->backup;
	b->l_INIT = v->l_INIT;
	b->l_b = *v->l_b;
	b->l_g = *v->l_g;
	b->l_max = *v->l_max;
	b->l_position = *v->l_position;
	b->l_r = *v->l_r;
	b->l_strip = *v->l_strip;
}
Value *cw_DIGITALLED_lookup(struct cw_DIGITALLED *m, int symbol) {
	if (symbol == sym_b) return &m->b;
	if (symbol == sym_g) return &m->g;
	if (symbol == sym_max) return &m->max;
	if (symbol == sym_r) return &m->r;
	if (symbol == sym_position) return &m->position;
	return 0;
}
MachineBase *cw_DIGITALLED_lookup_machine(struct cw_DIGITALLED *m, int symbol) {
	if (symbol == sym_strip) return m->_strip;
	return 0;
}
void cw_DIGITALLED_describe(struct cw_DIGITALLED *m);
int cw_DIGITALLED_handle_message(struct MachineBase *ramp, struct MachineBase *machine, int state);
int cw_DIGITALLED_check_state(struct cw_DIGITALLED *m);
struct cw_DIGITALLED *create_cw_DIGITALLED(const char *name, MachineBase *strip, int position) {
	struct cw_DIGITALLED *p = (struct cw_DIGITALLED *)malloc(sizeof(struct cw_DIGITALLED));
	Init_cw_DIGITALLED(p, name, strip, position);
	return p;
}
int cw_DIGITALLED_black(struct cw_DIGITALLED *m, ccrContParam) {
	//struct cw_DIGITALLED_Vars_backup *v;
	//v = m->backup;
#if DEBUG_LOG
	ESP_LOGI(TAG, "cw_DIGITALLED_black");
#endif
	m->machine.set_value(&m->machine, "r", m->vars->l_r,0);
	m->machine.set_value(&m->machine, "g", m->vars->l_g,0);
	m->machine.set_value(&m->machine, "b", m->vars->l_b,0);
	m->machine.execute = 0;
	return 1;
}
int cw_DIGITALLED_blue(struct cw_DIGITALLED *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "cw_DIGITALLED_blue");
#endif
	struct cw_DIGITALLED_Vars_backup *v;
	v = m->backup;
	m->machine.set_value(&m->machine, "r", m->vars->l_r,0);
	m->machine.set_value(&m->machine, "g", m->vars->l_g,0);
	m->machine.set_value(&m->machine, "b", m->vars->l_b,v->l_max);
	m->machine.execute = 0;
	return 1;
}
int cw_DIGITALLED_green(struct cw_DIGITALLED *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "cw_DIGITALLED_green");
#endif
	struct cw_DIGITALLED_Vars_backup *v;
	v = m->backup;
	m->machine.set_value(&m->machine, "r", m->vars->l_r,0);
	m->machine.set_value(&m->machine, "g", m->vars->l_g,v->l_max);
	m->machine.set_value(&m->machine, "b", m->vars->l_b,0);
	m->machine.execute = 0;
	return 1;
}
int cw_DIGITALLED_red(struct cw_DIGITALLED *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "cw_DIGITALLED_red");
#endif
	struct cw_DIGITALLED_Vars_backup *v;
	v = m->backup;
	m->machine.set_value(&m->machine, "r", m->vars->l_r,v->l_max);
	m->machine.set_value(&m->machine, "g", m->vars->l_g,0);
	m->machine.set_value(&m->machine, "b", m->vars->l_b,0);
	m->machine.execute = 0;
	return 1;
}
int cw_DIGITALLED_white(struct cw_DIGITALLED *m, ccrContParam) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "cw_DIGITALLED_white");
#endif
	struct cw_DIGITALLED_Vars_backup *v;
	v = m->backup;
	m->machine.set_value(&m->machine, "r", m->vars->l_r,v->l_max);
	m->machine.set_value(&m->machine, "g", m->vars->l_g,v->l_max);
	m->machine.set_value(&m->machine, "b", m->vars->l_b,v->l_max);
	m->machine.execute = 0;
	return 1;
}
int cw_DIGITALLED_INIT_enter(struct cw_DIGITALLED *m, ccrContParam) {
	m->machine.execute = 0;
	return 1;
};
int cw_DIGITALLED_handle_message(struct MachineBase *obj, struct MachineBase *source, int message) {
#if DEBUG_LOG
	ESP_LOGI(TAG, "cw_DIGITALLED_handle_message");
#endif
	//struct cw_DIGITALLED *m = (struct cw_DIGITALLED *)obj;
	if (message == cw_message_black)
		MachineActions_add(obj, (enter_func)cw_DIGITALLED_black);
	if (message == cw_message_blue)
		MachineActions_add(obj, (enter_func)cw_DIGITALLED_blue);
	if (message == cw_message_green)
		MachineActions_add(obj, (enter_func)cw_DIGITALLED_green);
	if (message == cw_message_red)
		MachineActions_add(obj, (enter_func)cw_DIGITALLED_red);
	if (message == cw_message_white)
		MachineActions_add(obj, (enter_func)cw_DIGITALLED_white);
	markPending(obj);
	return 1;
}
void Init_cw_DIGITALLED(struct cw_DIGITALLED *m, const char *name, MachineBase *strip, int position) {
	initMachineBase(&m->machine, name);
	m->machine.class_name = "DIGITALLED";
	init_io_address(&m->addr, 0, 0, 0, 0, iot_none, IO_STABLE);
	m->_strip = strip;
	if (strip) MachineDependencies_add(strip, cw_DIGITALLED_To_MachineBase(m));
	m->position = position;
	m->b = 0;
	m->g = 0;
	m->max = 32;
	m->r = 0;
	m->machine.state = state_cw_INIT;
	m->machine.check_state = ( int(*)(MachineBase*) )cw_DIGITALLED_check_state;
	m->machine.handle = (message_func)cw_DIGITALLED_handle_message; // handle message from other machines
	m->machine.lookup = (lookup_func)cw_DIGITALLED_lookup; // lookup symbols within this machine
	m->machine.lookup_machine = (lookup_machine_func)cw_DIGITALLED_lookup_machine; // lookup symbols within this machine
	m->machine.describe = (describe_func)cw_DIGITALLED_describe;
	m->vars = (struct cw_DIGITALLED_Vars *)malloc(sizeof(struct cw_DIGITALLED_Vars));
	m->backup = (struct cw_DIGITALLED_Vars_backup *)malloc(sizeof(struct cw_DIGITALLED_Vars_backup));
	init_Vars(m, m->vars);
	backup_Vars(m);
	MachineActions_add(cw_DIGITALLED_To_MachineBase(m), (enter_func)cw_DIGITALLED_INIT_enter);
	markPending(&m->machine);
}
struct IOAddress *cw_DIGITALLED_getAddress(struct cw_DIGITALLED *p) {
	return (p->addr.io_type == iot_none) ? 0 : &p->addr;
}
MachineBase *cw_DIGITALLED_To_MachineBase(struct cw_DIGITALLED *p) { return &p->machine; }

int cw_DIGITALLED_check_state(struct cw_DIGITALLED *m) {
	//struct cw_DIGITALLED_Vars_backup *v = m->backup;
	int res = 0;
	int new_state = 0; enter_func new_state_enter = 0;
	backup_Vars(m);
	if (new_state && new_state != m->machine.state) {
		changeMachineState(cw_DIGITALLED_To_MachineBase(m), new_state, new_state_enter); // TODO: fix me
		markPending(&m->machine);
		res = 1;
	}
	return res;
}
void cw_DIGITALLED_describe(struct cw_DIGITALLED *m) {
	//struct cw_DIGITALLED_Vars_backup *v = m->backup;
{
	char buf[100];
	snprintf(buf, 100, "%s: %s  Class: DIGITALLED", m->machine.name, name_from_id(m->machine.state));
	sendMQTT(0, "/response", buf);
	snprintf(buf, 100, "Timer: %ld", m->machine.TIMER);
	sendMQTT(0,"/response", buf);
	snprintf(buf, 100, "colour: %d,%d,%d position: %d\n", m->r, m->g, m->b, m->position);
	sendMQTT(0,"/response", buf);
}
}
