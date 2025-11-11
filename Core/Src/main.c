#include "init.h"
#include "FDCAN.h"

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_FDCAN1_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	FDCAN_Config();

//	load_flash_params();

	while (1)
	{
	}
}
