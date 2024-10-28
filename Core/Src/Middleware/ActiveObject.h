#ifndef __ACTIVEOBJECT_H
#define __ACTIVEOBJECT_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct {
    uint16_t signal;
} Event;

typedef struct {
    struct AOvtable const * vptr; /* Virtual pointer */

    TaskHandle_t CreatedTask;
    QueueHandle_t Queue;
} ActiveObject;

/**
 * Bảng hàm ảo
 */
struct AOvtable {
    void (*create) (ActiveObject * const self,
                    uint16_t StackSize,
                    uint8_t Priority,
                    UBaseType_t QueueLength);
    void (*loopEvent) (void *pvParameters);
    void (*post) (ActiveObject const * const self, Event const * const event);
    void (*postfromISR) (ActiveObject const * const self, Event const * const event, BaseType_t * pxHigherPriorityTaskWoken);
};

void AO_Init (ActiveObject * const self);

#endif /* __ACTIVEOBJECT_H */