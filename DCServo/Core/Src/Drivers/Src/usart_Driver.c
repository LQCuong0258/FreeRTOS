#include "usart_Driver.h"
#include "FreeRTOS.h"
#include "queue.h"

UART_HandleTypeDef huart1;

extern QueueHandle_t UARTQueue;

char Buffer[100];
uint8_t BufferIndex = 0;

void USART1_IRQHandler(void)
{
	uint8_t ReceivedData;
	HAL_UART_Receive(&huart1, &ReceivedData, 1, HAL_MAX_DELAY);

	if(ReceivedData == 'S')
	{ 
		Buffer[BufferIndex] = '\0';

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(UARTQueue, Buffer, &xHigherPriorityTaskWoken);

		BufferIndex = 0;
		memset(Buffer, '\0', sizeof(Buffer));

		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
	else Buffer[BufferIndex++] = ReceivedData;

	// __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
}

void SendString(char* str)
{
    uint16_t length = strlen(str);

    HAL_UART_Transmit(&huart1, (uint8_t*)str, length, HAL_MAX_DELAY);
}


void USART1_Init(void)
{
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Cấu hình PA9 (USART1_TX)
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Cấu hình PA10 (USART1_RX)
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;                       // Tốc độ baud
    huart1.Init.WordLength = UART_WORDLENGTH_8B;         // Độ dài từ: 8 bit
    huart1.Init.StopBits = UART_STOPBITS_1;              // 1 bit stop
    huart1.Init.Parity = UART_PARITY_NONE;               // Không có parity
    huart1.Init.Mode = UART_MODE_TX_RX;                  // Chế độ truyền và nhận
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;         // Không sử dụng kiểm soát luồng
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;     // Lấy mẫu quá mức

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        // Xử lý lỗi nếu cần
        Error_Handler();
    }

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

	HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}