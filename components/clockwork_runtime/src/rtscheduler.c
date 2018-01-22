//
// rtscheduler.h.c
// Created by Martin on 18/10/2016.


#include <limits.h>

#include "rtscheduler.h"
#include "runtime.h"
//#include <stdio.h>
#include <esp_log.h>
#include <ccan/list/list.h>

#define DEBUG_LOG 0

static const char* TAG = "RTScheduler";

static struct RTScheduler *scheduler = 0;

struct Schedule;
struct RTScheduler {
	struct list_head schedule;
	int processing;
	int done;
	SemaphoreHandle_t scheduler_mutex;
};

struct Runnable {
    struct Runnable *next;
    MachineBase *machine;
};

static struct Runnable *runnable = 0;

void pushRunnable(MachineBase *m) {
    struct Runnable *item = (struct Runnable*)malloc(sizeof(struct Runnable));
    item->next = runnable;
    item->machine = m;
    runnable = item;
}

struct RTScheduler *RTScheduler_get() {
    assert(scheduler);
    BaseType_t res = xSemaphoreTake(scheduler->scheduler_mutex, 1);
    if (res == pdPASS)
        return scheduler;
    else {
        ESP_LOGI(TAG,"RTScheduler_get failed");
        return 0;
    }
}
void RTScheduler_release() {
    assert(scheduler);
    BaseType_t res = xSemaphoreGive(scheduler->scheduler_mutex);
    if (res == pdFALSE) {
        ESP_LOGI(TAG,"RTScheduler_release failed to give");
    }
}

SemaphoreHandle_t RTScheduler_mutex(struct RTScheduler *s) { if (!s) return 0; else return s->scheduler_mutex; }

/* run queue */

// typedef struct Schedule {
//  struct Schedule *next;
//  struct ScheduleItem *item;
// } Schedule;

struct ScheduleItem *ScheduleItem_create(unsigned long trigger_time, struct MachineBase *machine) {
    struct ScheduleItem *item = (struct ScheduleItem *)malloc(sizeof(*item));
	item->active = 1;
	item->trigger_time = upTime() + trigger_time;
	item->machine = machine;
	return item;
}


void RTScheduler_free(struct RTScheduler *scheduler, struct ScheduleItem *item) {
    free(item);
}

void RTScheduler_add(struct RTScheduler *scheduler, struct ScheduleItem *item) {
    list_add(&scheduler->schedule, &item->list);
}

struct RTScheduler *RTScheduler_create() {
    struct RTScheduler *scheduler = (struct RTScheduler *)malloc(sizeof(struct RTScheduler));
    assert(scheduler);
	list_head_init(&scheduler->schedule);
	scheduler->processing = 0;
	scheduler->done = 0;
	scheduler->scheduler_mutex = xSemaphoreCreateMutex();
    assert(scheduler->scheduler_mutex);
	return scheduler;
}
//
// unsigned int RTScheduler_size(struct RTScheduler *scheduler) {
//  struct Schedule *s = scheduler->schedule;
//  unsigned int count = 0; while (s) { s = s->next; ++count; }
//  return count;
// }

/* check if there is an item that is ready to trigger */
#if 0
int RTScheduler_check(struct RTScheduler *ctx) {
  Schedule *iter;
  unsigned long now;
	if (ctx->processing) return 1; /* busy processing in the idle() loop */
	if (ctx->schedule == 0) return 0; /* no items */
	iter = ctx->schedule;
	now = upTime();
	while (iter) {
		if (iter->item->trigger_time >= now) return 1;
		iter = iter->next;
	}
	return 0;
}
#endif

void RTSchedulerTask(void *pvParameter) {
    scheduler = RTScheduler_create();
    unsigned long now;

    //min_time = LONG_MAX;
	scheduler->processing = 0;
    while (!scheduler_sem || !process_sem) vTaskDelay(5);

	while (1) {
        if (scheduler->processing) {
            // tell main we are done
            while (xSemaphoreGive(process_sem) == pdFALSE) {
                ESP_LOGI(TAG,"scheduler failed to give to main");
                taskYIELD();
            }
            scheduler->processing = 0;
        }

        // wait for permission to run
        BaseType_t res = xSemaphoreTake( scheduler_sem, 10 );
        if (res != pdPASS) {
            taskYIELD();
            continue;
        }
    	scheduler->processing = 1;
        now = upTime();
        if (xSemaphoreTake(scheduler->scheduler_mutex, 1) == pdFALSE) {
#if DEBUG_LOG
            ESP_LOGI(TAG,"scheduler could not get mutex");
#endif
            continue;
        }

        struct ScheduleItem *item, *next;
        list_for_each_safe(&scheduler->schedule, item, next, list) {
#if DEBUG_LOG
            int count = 0;
            ESP_LOGI(TAG, "checking %d %s (%ld)", ++count, item->machine->name, item->trigger_time);
#endif
            if (item->active && item->trigger_time <= now) {
#if DEBUG_LOG
                ESP_LOGI(TAG,"%lld [%d]triggering item", upTime(), item->machine->id);
#endif
        		if (item->machine) { pushRunnable(item->machine); }
        		item->active = 0;
                list_del(&item->list);
                free(item);
        	}
    	}
        xSemaphoreGive(scheduler->scheduler_mutex);
        while (runnable) {
            markRunnable(runnable->machine);
            struct Runnable *next = runnable->next;
            free(runnable);
            runnable = next;
        }
	}
}

