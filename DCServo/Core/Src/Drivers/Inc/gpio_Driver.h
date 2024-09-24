/* 
 * Author: Syaoran
 * Created on: 2024-09-17
 */
#ifndef __GPIO_DRIVER_H
#define __GPIO_DRIVER_H

#include "main.h"

typedef enum {
	CW,
	CCW
} RotaMode_t;

void LED_PC13(void);
void Rotation_Init(void);
void RotationMode(RotaMode_t rota);

#endif /* __GPIO_DRIVER_H */