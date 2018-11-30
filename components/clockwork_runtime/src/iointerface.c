/*
    iointerface

    The io interface uses its own task to collect io status and store the results into a memory map. As part of its processing
    cycle it collects an output data map from clockwork and where there are differences, writes the differences to
    the io.

    The main processing loop periodically collects the io map and generate change events for the clockwork devices.
*/
#include "iointerface.h"
#include <freertos/task.h>
#include <esp_log.h>
#include "rtio.h"
#include "runtime.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "cw_ANALOGOUTPUT.h"
#include "cw_ANALOGINPUT.h"

#define DEBUG_LOG 0
#if DEBUG_LOG
static const char* TAG = "IOInterface";
#endif

static uint8_t *io_map = 0;
static struct RTIOInterface *io_interface = 0;


struct RTIOInterface {
	struct list_head drivers;
	struct list_head io;
	int processing;
	int done;
	SemaphoreHandle_t io_interface_mutex;
};

struct DriverItem {
    struct list_node list;
	unsigned int data;
};

struct IOItem {
    struct list_node list;
	struct IOAddress *data;
    uint8_t gpio;
    MachineBase *machine;
};

struct RTIOInterface *RTIOInterface_create() {
    struct RTIOInterface *interface = (struct RTIOInterface *)malloc(sizeof(struct RTIOInterface));
    assert(interface);
	list_head_init(&interface->drivers);
	list_head_init(&interface->io);
	interface->processing = 0;
	interface->done = 0;
	interface->io_interface_mutex = xSemaphoreCreateMutex();
    assert(interface->io_interface_mutex);
	return interface;
}

struct RTIOInterface *RTIOInterface_get() {
    assert(io_interface);
    BaseType_t res = xSemaphoreTake(io_interface->io_interface_mutex, 1);
    if (res == pdPASS)
        return io_interface;
    else {
#if DEBUG_LOG
        ESP_LOGI(TAG,"RTIOInterface_get failed");
#endif
        return 0;
    }
}

void RTIOInterface_release() {
    assert(io_interface);
    BaseType_t res = xSemaphoreGive(io_interface->io_interface_mutex);
    if (res == pdFALSE) {
#if DEBUG_LOG
        ESP_LOGI(TAG,"RTIOInterface_get failed to give");
#endif
    }
}

SemaphoreHandle_t RTIOInterface_mutex(struct RTIOInterface *io) { if (!io) return 0; else return io->io_interface_mutex; }

void initDrivers(struct RTIOInterface *interface) {
    struct DriverItem *item, *next;
    list_for_each_safe(&interface->drivers, item, next, list) {
    }
}

struct IOItem *IOItem_create(MachineBase *m, struct IOAddress *addr, uint8_t gpio_pin) {
    struct IOItem *item = (struct IOItem *)malloc(sizeof(*item));
	IOItem_init(m, item, addr, gpio_pin);
	return item;
}

void IOItem_init(MachineBase *m, struct IOItem *item, struct IOAddress *addr, uint8_t gpio_pin) {
    item->data = addr;
    item->gpio = gpio_pin;
    item->machine = m;
}

void RTIOInterface_add(struct RTIOInterface *io_interface, struct IOItem *item) {
    list_add(&io_interface->io, &item->list);
}

void RTIOInterface_free(struct RTIOInterface *io_interface, struct IOItem *item) {
    free(item);
}

/* loop through the io items and update the device outputs to match the clockwork state */
void writeIO() {
    struct IOItem *item, *next;
    list_for_each_safe(&io_interface->io, item, next, list) {
        if (item->data->io_type == iot_digout) {
            if (item->data->status == IO_PENDING) {
                uint8_t cw_val = rt_get_io_bit(item->data);
                //uint8_t val = gpio_get_level(item->gpio);
                if (item->data->io_type == iot_digout) {
                    gpio_set_level(item->gpio, cw_val);
#if DEBUG_LOG
                    //ESP_LOGI(TAG,"writeIO gpio set level of %d:%d (pin %d) to %d", item->data->io_offset, item->data->io_bitpos, item->gpio, cw_val);
#endif
                    item->data->status = IO_DONE;
                    if (item->machine) markPending(item->machine);
                }
            }
        }
        else if (item->data->io_type == iot_pwm) {
            if (item->data->status == IO_PENDING) {
                uint16_t cw_val = rt_get_io_uint16(item->data);
                struct cw_ANALOGOUTPUT *a_out = (struct cw_ANALOGOUTPUT*)item->machine;
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, cw_ANALOGOUTPUT_get_channel(a_out), cw_val);
                ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
#if DEBUG_LOG
                //ESP_LOGI(TAG,"writeIO gpio set level of %d:%d (pin %d) to %d", item->data->io_offset, item->data->io_bitpos, item->gpio, cw_val);
#endif
                item->data->status = IO_DONE;
                if (item->machine) markPending(item->machine);
            }
        }

    }
}

/* loop through the io items and update clockwork state to match the device status */
void readIO() {
    struct IOItem *item, *next;
    list_for_each_safe(&io_interface->io, item, next, list) {
        if (item->data->io_type == iot_digin) { // always read inputs
            uint8_t val = gpio_get_level(item->gpio);
            uint8_t cw_val = rt_get_io_bit(item->data);
            if (val != cw_val) {
                rt_set_io_bit(item->data, val);
                item->data->status = IO_DONE;
                if (item->machine) markPending(item->machine);
#if DEBUG_LOG
                ESP_LOGI(TAG,"readIO gpio change to %d done", val);
#endif
                 // TBD raise event
                markPending(item->machine);
            }
        }
        if (item->data->io_type == iot_adc) { // always read inputs
            struct cw_ANALOGINPUT *ain = (struct cw_ANALOGINPUT*)item->machine;
            int val = adc1_get_raw(cw_ANALOGINPUT_getChannel(ain));
            uint8_t cw_val = rt_get_io_bit(item->data);
            if (val != cw_val) {
                rt_set_io_uint16(item->data, val);
                item->data->status = IO_DONE;
                if (item->machine) markPending(item->machine);
                // TBD raise event
            }
        }
        else if (item->data->io_type == iot_digout) { // read outputs but only update clockwork if output matches current state
            uint8_t val = gpio_get_level(item->gpio);
            uint8_t cw_val = rt_get_io_bit(item->data);
            if (val == cw_val && item->data->status == IO_PENDING) {
#if DEBUG_LOG
                ESP_LOGI(TAG,"readIO gpio change to %d done", cw_val);
#endif
                item->data->status = IO_DONE;
                if (item->machine) markPending(item->machine);
                // TBD raise event
            }
        }
    }
}

void createIOMap() {

    if (io_map) free(io_map);
#if DEBUG_LOG
    ESP_LOGI(TAG,"%lld creating IO map", upTime());
#endif

    struct IOItem *item, *next;
    unsigned int bits = 0, bit_offset = 0;
    unsigned int bytes = 0, byte_offset = 0;
    int count = 0;
    list_for_each_safe(&io_interface->io, item, next, list) {
        ++count;
        if (item->data->bitlen == 1) {
            if (bits == 0) { bit_offset = byte_offset++; }
            item->data->io_offset = bit_offset;
            item->data->io_bitpos = (bits++ % 8);
            if (bits % 8 == 0) { ++bit_offset; bits = 0; }
#if DEBUG_LOG
            ESP_LOGI(TAG, "%lld added io bit at %d:%d for gpio: %d",upTime(), item->data->io_offset, item->data->io_bitpos, item->gpio);
#endif
        }
        else if (item->data->bitlen == 8) {
            ++bytes;
            item->data->io_offset = byte_offset;
            item->data->io_bitpos = 0;
            ++byte_offset;
#if DEBUG_LOG
            ESP_LOGI(TAG, "%lld added io byte at %d:%d",upTime(), item->data->io_offset, item->data->io_bitpos);
#endif
        }
        else if (item->data->bitlen == 16) {
            bytes += 2;
            item->data->io_offset = byte_offset;
            item->data->io_bitpos = 0;
            byte_offset += 2;
#if DEBUG_LOG
            ESP_LOGI(TAG, "%lld added io word at %d:%d",upTime(), item->data->io_offset, item->data->io_bitpos);
#endif
        }
        else if (item->data->bitlen == 32) {
            bytes += 4;
            item->data->io_offset = byte_offset;
            item->data->io_bitpos = 0;
            byte_offset += 4;
#if DEBUG_LOG
            ESP_LOGI(TAG, "%lld added io long at %d:%d",upTime(), item->data->io_offset, item->data->io_bitpos);
#endif
        }
    }
    unsigned int len = bits/8 + 1 + bytes;
    io_map = (uint8_t *)malloc(len);
    memset(io_map, 0, len);
#if DEBUG_LOG
    ESP_LOGI(TAG, "%lld io map is %d bytes long for %d items",upTime(), len, count);
#endif
}

void RTIOInterface(void *pvParameter) {
    io_interface = RTIOInterface_create();
    while (!scheduler_sem || !process_sem) vTaskDelay(5);
    initDrivers(io_interface);
    while (!io_map) vTaskDelay(1);

	io_interface->processing = 0;

	while (1) {
        writeIO();
        readIO();
        vTaskDelay(1);
        taskYIELD();
    }
}

void registerIO(struct MachineBase *m, struct IOAddress *a) {

}
