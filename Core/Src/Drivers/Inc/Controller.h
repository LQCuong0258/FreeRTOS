#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

typedef struct {
  float pwm;
  float setpoint;
  float velocity;
  float position;
} data;



void PID_Vel(data* value, uint8_t ts);

#endif /* __CONTROLLER_H */