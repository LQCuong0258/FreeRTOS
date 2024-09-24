#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


QueueHandle_t EncoderQueue;
QueueHandle_t UARTQueue;

void EncoderTask(void* vParameters);
void ControllerTask(void* vParametes);
void CommunicationTask(void* vParameters);


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  USART1_Init();
  Encoder_Init();
  PWM_init();

  LED_PC13();

  EncoderQueue = xQueueCreate(1, sizeof(int16_t));
  UARTQueue = xQueueCreate(1, sizeof(char)*100);

  xTaskCreate(EncoderTask, NULL, configMINIMAL_STACK_SIZE, NULL, 4, NULL);
  xTaskCreate(ControllerTask, NULL, configMINIMAL_STACK_SIZE, NULL, 3, NULL);
  xTaskCreate(CommunicationTask, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);


  vTaskStartScheduler();


  while (1)
  {}
}

uint32_t PWM = 0;
char BufferReceive[100];
void ControllerTask(void* vParametes)
{
  TickType_t MSDelay = pdMS_TO_TICKS(100);
	for(;;)
	{
    if (xQueueReceive(UARTQueue, &BufferReceive, pdMS_TO_TICKS(400)) == pdTRUE)
		{
      PWM = atoi(BufferReceive);
      RotationMode(CW);
			PWM_Set(PWM);

		}
    else
    {
      SendString("Controller Task\n");
    }
	}
}

int16_t countSend = 0;
void EncoderTask(void* vParametes)
{
	TickType_t MSDelay = pdMS_TO_TICKS(9);
	for(;;)
	{
		countSend = Get_Encoder();

		xQueueSend(EncoderQueue, &countSend, MSDelay);
		// xSemaphoreTake(EncoderSemaphore, portMAX_DELAY);

	}
}

char buffer[100];
int16_t countReceived = 0;
void CommunicationTask(void* vParametes)
{
	for(;;)
	{
		if (xQueueReceive(EncoderQueue, &countReceived, portMAX_DELAY) == pdTRUE)
		{
			sprintf(buffer, "Encoder: %d\n", countReceived);
			SendString(buffer);

      vTaskDelay(pdMS_TO_TICKS(1000));
		}
		// xSemaphoreTake(CommunicationSemaphore, portMAX_DELAY);

    // SendString("Hello World\n");
    
	}
}
