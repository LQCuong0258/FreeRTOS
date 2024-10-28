#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

QueueHandle_t CommuniQueue;
SemaphoreHandle_t CommuniSemaphore;

uint8_t pwm;
extern int16_t encoder, pre_encoder;
extern int16_t delta_encoder;
float Setpoint, velocity, position;
uint8_t Ts = 100;

typedef struct {
  float setpoint;
  float velocity;
} data;

void PID_Vel(data* value);

/**
 * 
 */
void MainTask(void * xTaskParameters) {
  data dataToSend;
  for(;;) {
    dataToSend.setpoint = Setpoint;
    dataToSend.velocity = delta_encoder * 60 / (Ts * 0.001 * 4.0 * 11 * 45);
    pre_encoder = encoder;
    // position = encoder * 360.0 / (4.0 * 11 * 45);
    PID_Vel(&dataToSend);

    xQueueSend(CommuniQueue, (void *)&dataToSend, portMAX_DELAY);
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
      sprintf(buffer, "%0.2f,%0.2f\n", dataToReceive.setpoint, dataToReceive.velocity);
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

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);	
  /* USER CODE BEGIN 2 */

  CommuniQueue = xQueueCreate(10, sizeof(data));
  vSemaphoreCreateBinary(CommuniSemaphore);

  xTaskCreate(MainTask, NULL, configMINIMAL_STACK_SIZE, NULL, 3, NULL);
  xTaskCreate(CommunicationTask, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL);

  vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  while (1) {}
}

void PID_Vel(data* value) {
  float error = value->setpoint - value->velocity;
  static float up, ui, errorReset;
  int64_t pwm;

  int64_t Hlim = 100;
  int64_t Llim = -100;

  float Kp = 0.7397;
  float Ki = 8.0296;
  float Kb = 0;
  
  up = Kp * error;
  ui += Ki * error * Ts * 0.001 + Kb * errorReset * Ts;

  int64_t uout = (int64_t)(up + ui);

  if (uout > 0) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

    if (uout > Hlim) pwm = Hlim;
    else pwm = uout;

    errorReset = pwm - uout;
  }
  else if (uout < 0) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

    if (uout < Hlim) pwm = Llim;
    else pwm = uout;

    errorReset = pwm - uout;

    uout *= -1;
  }
  else {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);    
    pwm = 0;

    errorReset = pwm - uout;
  }

  TIM3->CCR1 = pwm;
}
