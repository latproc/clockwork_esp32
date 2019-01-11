#ifndef __CW_runtime_h__
#define __CW_runtime_h__

//#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <freertos/semphr.h>
#include <ccan/list/list.h>
#include <inttypes.h>
#include "coroutine.h"
#include "mqtt_client.h"

#define FLAG_PASSIVE 0x1
#define FLAG_SLEEPING 0x2
#define MASK_PASSIVE 0xfe
#define MASK_SLEEPING 0xfd
#define state_INIT 0
#define UNDEFINED_STATE -1

// standard messages
#define cw_message_turnOff -100
#define cw_message_turnOn -101
#define cw_message_property_change -102

// standard symbols
#define sym_VALUE 1
#define sym_broker 2
#define sym_message 3

extern int network_is_connected;

extern SemaphoreHandle_t scheduler_sem;
extern SemaphoreHandle_t process_sem;
extern SemaphoreHandle_t runtime_mutex;
extern SemaphoreHandle_t io_interface_sem;

void rt_init(void);

void cwrt_setup(void);
void cwrt_process(unsigned long *);

struct MachineBase;
struct cw_MQTTBROKER;

typedef int (*enter_func)(struct MachineBase *, ccrContParam);
typedef int (*message_func)(struct MachineBase *, struct MachineBase *, int state);
typedef int* (*lookup_func)(struct MachineBase *, int symbol);
typedef struct MachineBase* (*lookup_machine_func)(struct MachineBase *, int symbol);
typedef void (*describe_func)(struct MachineBase *);
typedef void (*set_value_func)(struct MachineBase *, const char *, int *, int);

typedef struct MachineBase {
	struct MachineBase *p_next;
	struct list_head depends;
	struct list_head actions;
	struct list_head messages;
    const char *name;
    unsigned int id;
	unsigned char flags;
	unsigned long START, TIMER;
	unsigned int state;
    ccrContext ctx;
	void (*init)();
	int (*executing)(struct MachineBase *);
	enter_func execute; // the currently executing action
	message_func handle; // handle messages
	lookup_func lookup; // lookup symbols
	lookup_machine_func lookup_machine; // lookup symbols
	describe_func describe; // describe the current state
	set_value_func set_value; // set a property value
	int (*check_state)(struct MachineBase *);
} MachineBase;

int haveMQTT();
void setMQTTclient(esp_mqtt_client_handle_t client);
void setMQTTstate(int which);
void publish_MQTT(struct cw_MQTTBROKER *broker, MachineBase *m, int state);
void publish_MQTT_property(struct cw_MQTTBROKER *broker, MachineBase *m, const char *name, int value);
void sendMQTT(struct cw_MQTTBROKER *broker, const char *topic, const char *data);

void receiveMQTT(struct cw_MQTTBROKER *context, const char *topic, size_t topic_len, const char *data, size_t len);
int have_external_message();
void process_next_external_message();

struct MachineListItem {
    struct list_node list;
	struct MachineBase *machine;
};

struct ActionListItem {
    struct list_node list;
	enter_func action;
};

struct MessageListItem {
	struct list_node list;
	struct MachineBase *from;
	int message;
};

MachineBase *getMachineIterator();
MachineBase *nextMachine(MachineBase *);

void initMachineBase(MachineBase*, const char *name);
void resetMachineBase(MachineBase *m);

int runnableMachines(); // are there any runnable machines?
void markRunnable(MachineBase *m);
MachineBase *nextRunnable();
void markPending(MachineBase *m);
void activatePending();
void goto_sleep(MachineBase *m);
void wake_up(MachineBase *m);
int is_asleep(MachineBase *m);

int stateCheckMachines(); // are there machines that need state rules evaluated?
void markStateCheck(MachineBase *m);
MachineBase *nextStateCheck();

void push_command(char *cmd);
int have_command();
char *pop_command();

void changeMachineState(struct MachineBase *, int new_state, enter_func handler);

void MachineDependencies_add(struct MachineBase *machine, struct MachineBase *item);
void MachineActions_add(struct MachineBase *machine, enter_func f);
void NotifyDependents_state_change(struct MachineBase *machine, int state); // this machine changed state
void NotifyDependents(struct MachineBase *machine); // this machine changed a property

uint64_t upTime();
void delay(unsigned long usec);
void debug(const char *);
void debugInt(const char *, int);

void cw_send(struct MachineBase *to, struct MachineBase *from, int message);

#endif
