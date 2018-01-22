#ifndef __iointerface_h__
#define __iointerface_h__

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <ccan/list/list.h>
#include "rtio.h"
#include "runtime.h"

void RTIOInterface(void *pvParameter);

struct RTIOInterface;
struct RTIOInterface *RTIOInterface_get();
void RTIOInterface_release();
SemaphoreHandle_t RTIOInterface_mutex(struct RTIOInterface *s);

void registerIO(struct MachineBase *, struct IOAddress *);
void createIOMap();

struct IOItem;
struct IOItem *IOItem_create(MachineBase *m, struct IOAddress *addr, uint8_t gpio_pin);
void IOItem_init(MachineBase *m, struct IOItem *item, struct IOAddress *addr, uint8_t gpio_pin);

void RTIOInterface_add(struct RTIOInterface *, struct IOItem *);
void RTIOInterface_free(struct RTIOInterface *, struct IOItem *);

#endif
