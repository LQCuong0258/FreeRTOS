#include "ActiveObject.h"

static void LoopEvent (void *pvParameters) {
    ActiveObject * AO_Instance = (ActiveObject *) pvParameters;
    while (1) {

    }
}

static void Create (ActiveObject * const self, uint16_t StackSize, uint8_t Priority, UBaseType_t QueueLength) {
    uint16_t StackDepth = StackSize / sizeof(StackType_t);

    self->Queue = xQueueCreate(QueueLength, sizeof(Event *));
    xTaskCreate(self->vptr->loopEvent, "ActiveObject", StackDepth, self, Priority + tskIDLE_PRIORITY, &self->CreatedTask);
}

static void Post (ActiveObject const * const self, Event const * const event) {
    xQueueSendToBack(self->Queue, (void *) &event, (TickType_t) 0);
}
static void PostFromISR (ActiveObject const * const self, Event const * const event, BaseType_t * pxHigherPriorityTaskWoken) {
    xQueueSendToBackFromISR(self->Queue, (void *) &event, pxHigherPriorityTaskWoken);
}

void AO_Init (ActiveObject * const self) {
    static const struct AOvtable vtable = {
        &Create,
        &LoopEvent,
        &Post,
        &PostFromISR
    };
    self->vptr = &vtable;
}