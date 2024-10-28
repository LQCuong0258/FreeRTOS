#ifndef USART_H
#define USART_H

#include "stm32f1xx_hal.h"
#include "string.h"

void USART1_Init(void);
void SendString(char* str);

#endif /* USART_H */