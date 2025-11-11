#include "init.h"
#include "FDCAN.h"
#include "dartt_params.h"
#include "fds.h"

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_FDCAN1_Init();
	MX_USART2_UART_Init();
	FDCAN_Config();

	load_flash_params(&fs_alias);

	while (1)
	{

	}
}
