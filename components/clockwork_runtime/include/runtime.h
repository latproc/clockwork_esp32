#ifndef __CW_runtime_h__
#define __CW_runtime_h__

//#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <freertos/semphr.h>
#include <inttypes.h>
#include "coroutine.h"

#define FLAG_PASSIVE 0x1
#define MASK_PASSIVE 0xfe
#define state_INIT 0

extern SemaphoreHandle_t scheduler_sem;
extern SemaphoreHandle_t process_sem;
extern SemaphoreHandle_t runtime_mutex;
extern SemaphoreHandle_t io_interface_sem;

void rt_init(void);

void cwrt_setup(void);
void cwrt_process(unsigned long *);

struct MachineBase;
typedef int (*enter_func)(struct MachineBase *, ccrContParam);

typedef struct MachineBase {
	struct MachineBase *p_next;
    const char *name;
    unsigned int id;
	unsigned char flags;
	unsigned long START, TIMER;
	unsigned int state;
    ccrContext ctx;
	void (*init)();
	int (*executing)(struct MachineBase *);
	enter_func execute;
	int (*check_state)(struct MachineBase *);
} MachineBase;

MachineBase *getMachineIterator();
MachineBase *nextMachine(MachineBase *);

void initMachineBase(MachineBase*, const char *name);
void resetMachineBase(MachineBase *m);

int runnableMachines(); // are there any runnable machines?
void markRunnable(MachineBase *m);
MachineBase *nextRunnable();
void markPending(MachineBase *m);
void activatePending();

void changeMachineState(struct MachineBase *, int new_state, enter_func handler);

uint64_t upTime();
void delay(unsigned long usec);
void debug(const char *);
void debugInt(const char *, int);

#endif
