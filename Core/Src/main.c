#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define pi  3.1415926539

QueueHandle_t CommuniQueue;
SemaphoreHandle_t CommuniSemaphore;

uint8_t pwm;
extern int16_t encoder, pre_encoder;
extern int16_t delta_encoder;
float Setpoint, velocity, position;
uint8_t Ts = 50;

/**
 * 
 */
void MainTask(void * xTaskParameters) {
  data dataToSend;
  for(;;) {
    dataToSend.setpoint = Setpoint;
    // dataToSend.velocity = delta_encoder * 60 / (Ts * 0.001 * 4.0 * 11 * 45); /* RPM */
    dataToSend.velocity = delta_encoder*2*pi / (Ts * 0.001 * 4.0 * 11 * 45); /* rad/s */
    pre_encoder = encoder;
    dataToSend.angle = encoder*2*pi / (4.0 * 11 * 45);
    PID_Vel(&dataToSend, Ts);

    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    // TIM3->CCR1 = pwm;
    
    xQueueSendToBack(CommuniQueue, (void *)&dataToSend, portMAX_DELAY);
    xSemaphoreGive(CommuniSemaphore);

    vTaskDelay(pdMS_TO_TICKS(Ts));
  }
}

/**
 * 
 */
char buffer[100];
void CommunicationTask(void * xTaskParameters) {
  data dataToReceive;
  for(;;) {
    if (xSemaphoreTake(CommuniSemaphore, portMAX_DELAY) && xQueueReceive(CommuniQueue, &dataToReceive, portMAX_DELAY) == pdTRUE) {
      sprintf(buffer, "%0.2f,%0.2f\n", dataToReceive.setpoint, dataToReceive.angle);
      // sprintf(buffer, "%d,%0.2f\n", pwm, dataToReceive.velocity);
      SendString(buffer);
    }
  }
}

int main(void)
{ 
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  Encoder_Init();
  PWM_init();
  USART1_Init();
  /* USER CODE BEGIN 2 */

  CommuniQueue = xQueueCreate(5, sizeof(data));
  vSemaphoreCreateBinary(CommuniSemaphore);

  xTaskCreate(MainTask, NULL, configMINIMAL_STACK_SIZE, NULL, 3, NULL);
  xTaskCreate(CommunicationTask, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);

  vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  while (1) {}
}
