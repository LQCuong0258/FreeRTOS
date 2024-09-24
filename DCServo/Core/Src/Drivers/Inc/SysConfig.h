/* 
 * Author: Syaoran
 * Created on: 2024-09-17
 */
#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#include "stm32f1xx_hal.h"

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void Error_Handler(void);


#endif /* __SYSCONFIG_H */