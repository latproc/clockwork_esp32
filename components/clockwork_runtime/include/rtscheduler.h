//
// rtscheduler.h.h
// Created by Martin on 18/10/2016.

#ifndef _rtscheduler_h_
#define _rtscheduler_h_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <ccan/list/list.h>

struct RTScheduler;
struct MachineBase;

struct RTScheduler *RTScheduler_get();
void RTScheduler_release();
SemaphoreHandle_t RTScheduler_mutex(struct RTScheduler *s);

struct RTScheduler *RTScheduler_create();
//int RTScheduler_check(struct RTScheduler *);
void RTSchedulerTask(void *pvParameter);
unsigned int RTScheduler_size(struct RTScheduler *);

struct ScheduleItem {
    struct list_node list;
	unsigned int active;
	unsigned long trigger_time;
	struct MachineBase *machine;
};

struct ScheduleItem *ScheduleItem_create(unsigned long trigger_time, struct MachineBase *machine);
void ScheduleItem_init(struct ScheduleItem *item);

void RTScheduler_add(struct RTScheduler *, struct ScheduleItem *);
void RTScheduler_free(struct RTScheduler *, struct ScheduleItem *);

#endif