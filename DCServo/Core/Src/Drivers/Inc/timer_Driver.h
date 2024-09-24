/* 
 * Author: Syaoran
 * Created on: 2024-09-17
 */
#ifndef __TIMER_DRIVER_H
#define __TIMER_DRIVER_H

#include "main.h"

void TIMER4_Init(void);
void Encoder_Init(void);
int16_t Get_Encoder(void);

void PWM_Set(uint32_t dutyCycle);
void PWM_init(void);

#endif /* __TIMER_DRIVER_H */
