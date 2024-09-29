#include "timer_Driver.h"
#include "FreeRTOS.h"
#include "semphr.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;



void Base_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		// Enable Clock for Timer 2

	TIM2->PSC = 7200 - 1;
	TIM2->ARR = 100 - 1;
	TIM2->CR1 |= TIM_CR1_CEN;				// Enable counter Timer 2
	TIM2->DIER |= TIM_DIER_UIE;				// Update Interrupt
	NVIC_SetPriority(TIM2_IRQn, 8);			// Set priority 8
	NVIC_EnableIRQ(TIM2_IRQn);  			// Enable interrupts Timer 2
}

void Encoder_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;					
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	GPIOB->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_MODE7);		// Input mode Pin PB6, PB7
	GPIOB->CRL &= ~((3 << 26) | (3 << 30));					// Reset CNF
    GPIOB->CRL |= (GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1);		// Input floating
    GPIOB->ODR |= (GPIO_ODR_ODR6 | GPIO_ODR_ODR7);			// Pull-up for PB6 và PB7

	TIM4->PSC = 0;
    TIM4->ARR = 0xFFFF;										// Max range 65535

    TIM4->SMCR	|= 0x03;									// Encoder Mode 3 (Chanel 1&2)
    TIM4->CCMR1	|= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;		// Config Input IC1 on TI1, IC2 on TI2
	TIM4->CCMR1	&= ~(TIM_CCMR1_IC1PSC | TIM_CCMR1_IC2PSC);	// Preload and fast Unable
    TIM4->CCMR1	&= ~(TIM_CCMR1_IC1F | TIM_CCMR1_IC2F);    	// No Filter
    TIM4->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC2P);			// Failing edge cho TI1 & TI2
    TIM4->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);			// Enable Capture Compare
    TIM4->CNT = 0x0000;    									// Reset counter
    
	TIM4->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;			// Enable Input Capture
	NVIC_SetPriority(TIM4_IRQn, 7); 						// Set priority for TIM4 equal 7
	NVIC_EnableIRQ(TIM4_IRQn);								// Enable interrupt Timer 4
	TIM4->CR1 |= TIM_CR1_CEN;  								// Enable Timer
}

int16_t Get_Encoder(void)
{
    return (int16_t)((uint32_t) TIM4->CNT);
}

void PWM_init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_TIM3_CLK_ENABLE();
    
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 3600;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	Rotation_Init();
}

void PWM_Set(uint32_t dutyCycle)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, dutyCycle);
}



