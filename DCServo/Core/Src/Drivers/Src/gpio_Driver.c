#include "gpio_Driver.h"

void LED_PC13(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	GPIO_InitStruct.Mode	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin		= GPIO_PIN_13;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	GPIO_InitStruct.Speed	= GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Rotation_Init(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Mode	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin		= GPIO_PIN_0;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	GPIO_InitStruct.Speed	= GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


	GPIO_InitStruct.Mode	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin		= GPIO_PIN_1;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	GPIO_InitStruct.Speed	= GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
}

void RotationMode(float U)
{
	if ( U > 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);		
	}
	else if ( U < 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);		
	}
}