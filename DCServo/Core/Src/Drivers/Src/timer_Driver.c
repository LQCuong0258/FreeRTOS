#include "timer_Driver.h"
#include "FreeRTOS.h"
#include "semphr.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

// extern SemaphoreHandle_t ControllerSemaphore;
extern SemaphoreHandle_t EncoderSemaphore;
// extern SemaphoreHandle_t CommunicationSemaphore;



uint32_t tick4 = 0;
void TIM4_IRQHandler(void)
{
	if (__HAL_TIM_GET_FLAG(&htim4, TIM_FLAG_UPDATE) != RESET)
	{
		__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);

		xSemaphoreGiveFromISR(EncoderSemaphore, NULL);
		// xSemaphoreGiveFromISR(ControllerSemaphore, NULL);

		// tick4++;
		// if(tick4 >= 50){
		// 	tick4 = 0;
		// 	// Kích hoạt semaphore cho các tác vụ Encoder và Motor Control
			
		// 	xSemaphoreGiveFromISR(CommunicationSemaphore, NULL);
		// }	
	}
}

void TIMER4_Init(void)
{
	__HAL_RCC_TIM4_CLK_ENABLE();
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 7199;  // Prescaler để có tick 1ms từ clock 72MHz
    htim4.Init.Period = 99;       // 100 tick = 10ms
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim4);

    // Kích hoạt ngắt cho Timer 4
    HAL_NVIC_SetPriority(TIM4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    HAL_TIM_Base_Start_IT(&htim4);
}

void Encoder_Init(void)
{
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	TIM_Encoder_InitTypeDef sConfig = {0};
  	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 65535;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_FALLING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 0;
	sConfig.IC2Polarity = TIM_ICPOLARITY_FALLING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 0;
	if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}

  	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

int16_t Get_Encoder(void)
{
    return (int16_t)((uint32_t) TIM2->CNT);
}

void PWM_init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PA6     ------> TIM3_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    __HAL_RCC_TIM3_CLK_ENABLE();
    
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 72-1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 100;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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