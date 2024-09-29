#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define pi	3.141592654

uint16_t High_lim, Low_lim;
float Kp, Ki, Kd, Kb, Ts, alpha;

QueueHandle_t EncoderQueue;
QueueHandle_t UARTQueue;

SemaphoreHandle_t EncoderSemaphore;

// void EncoderTask(void* vParameters);
void ControllerTask(void* vParametes);
void PID(float SetPoint, float CurrentValue);
// void CommunicationTask(void* vParameters);

/**
 * Input Capture to read Encoder
 */
int16_t Counter;
void TIM4_IRQHandler(void)
{
    if ((TIM4->SR & TIM_SR_CC1IF) || (TIM4->SR & TIM_SR_CC2IF))
    {
        TIM4->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF);
		
		Counter = Get_Encoder();
    }
}


/**
 * 1s cycle interrupt then Controller Task works
 * 10ms cycle interrupt then Controller Task does not work
 */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;

		BaseType_t xHigherPriorityTaskWoken = pdFAIL;
		xQueueSendFromISR(EncoderQueue, &Counter, &xHigherPriorityTaskWoken);
		xSemaphoreGiveFromISR(EncoderSemaphore, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}



int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	USART1_Init();
	Base_Init();
	Encoder_Init();
	PWM_init();

	LED_PC13();

	// Queue Init
	EncoderQueue = xQueueCreate(20, sizeof(int16_t));
	UARTQueue = xQueueCreate(1, sizeof(char)*100);

	// Semaphore Init
	vSemaphoreCreateBinary(EncoderSemaphore);


	xTaskCreate(ControllerTask, NULL, configMINIMAL_STACK_SIZE, NULL, 4, NULL);
	// xTaskCreate(CommunicationTask, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL);


	vTaskStartScheduler();

	while (1)
	{}
}


/**
 * Controller Task calculates control value every time timer 2 interrupt sends semaphore
 */
float SetPoint, CurrentValue;
void ControllerTask(void* vParametes)
{
	int16_t CntReceive;
	// float CurrentValue;
	char buffer[100];
	for(;;)
	{
		xQueueReceive(EncoderQueue, &CntReceive, portMAX_DELAY);
		xSemaphoreTake(EncoderSemaphore, portMAX_DELAY);


		CurrentValue = CntReceive * 360.0 / (4.0 * 11 * 45);

		PID(SetPoint, CurrentValue);

		// sprintf(buffer, "SetPoint: %f; CurrentValue: %f\r\n", SetPoint, CurrentValue);
		// SendString(buffer);
	}
}


// void CommunicationTask(void* vParametes)
// {
// 	char buffer[100];
// 	for(;;)
// 	{
// 		if (xQueueReceive(UARTQueue, &buffer, portMAX_DELAY) == pdTRUE)
// 		{
// 			SendString(buffer);
// 			SendString("Communication Task!\r\n");
//       		vTaskDelay(pdMS_TO_TICKS(2000));
// 		}
// 	}
// }

void PID(float SetPoint, float CurrentValue)
{
	uint32_t pwm = 0;
	
	High_lim = 3600, Low_lim = 0;
	static float e_pre = 0, Ui_pre = 0, e_reset = 0, Ud_f_pre = 0;
	
	double Up, Ui, Ud, Ud_f;
	
	Kp = 259.0972;
	Ki = 1847.9848;
	Kd = 7.4299;
	Kb = 15.771;

	Ts = 0.01, alpha = 0.9;
	
	float Error = SetPoint - CurrentValue;		// error
	
	Up = Kp*Error;
	
	Ui = Ui_pre + Ki*Ts*Error + Kb*Ts*e_reset;	// Anti-Windup
	
	Ud = Kd*(Error - e_pre)/Ts;
	Ud_f = (1 - alpha)*Ud_f_pre + alpha*Ud;		// low pass fillter

	// Luu lai cac gia tri
	e_pre = Error;
	Ui_pre = Ui;
	Ud_f_pre = Ud_f;
	
	float U = Up + Ui + Ud_f;
	
	// set rotations
	RotationMode(U);

	uint32_t Uout = abs((int) U);

	// Saturated
	if (Uout > High_lim) pwm = High_lim;
	else if(Uout < Low_lim) pwm = Low_lim;

	e_reset = pwm - Uout;

	// Set value PWM
	PWM_Set(pwm);
}