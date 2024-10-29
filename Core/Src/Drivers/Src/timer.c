#include "timer.h"

/**
 * Input Capture to read Encoder
 */
int16_t encoder;
int16_t pre_encoder;
int16_t delta_encoder;
void TIM4_IRQHandler(void)
{
    if ((TIM4->SR & TIM_SR_CC1IF) || (TIM4->SR & TIM_SR_CC2IF))
    {
        TIM4->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF);
		
		encoder = (int16_t)((uint32_t) TIM4->CNT);

        delta_encoder = encoder - pre_encoder;
        
        if (delta_encoder > 32767) delta_encoder -= 65536;
        if (delta_encoder < -32768) delta_encoder += 65536;
    }
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
	NVIC_SetPriority(TIM4_IRQn, 7); 						// Set priority for TIM4
	NVIC_EnableIRQ(TIM4_IRQn);								// Enable interrupt Timer 4
	TIM4->CR1 |= TIM_CR1_CEN;  								// Enable Timer
}

void PWM_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6);        // Clear pin
    GPIOA->CRL |= (GPIO_CRL_MODE6_1 | GPIO_CRL_MODE6_0);    // Output mode, max speed 50 MHz
    GPIOA->CRL |= GPIO_CRL_CNF6_1;                        // Alternate function push-pull

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 36-1;       // Prescaler tần số Timer 2MHz (72 MHz / 36)
    TIM3->ARR = 100-1;      // Tần số PWM là 20kHz
    TIM3->CCR1 = 0;

    TIM3->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM3->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos);   // PWM mode 1
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;             // Enable preload register for CCR1
    TIM3->CCER |= TIM_CCER_CC1E;
    TIM3->CR1 |= TIM_CR1_CEN;                   // Enable Timer 3
    TIM3->EGR |= TIM_EGR_UG;

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);	
}