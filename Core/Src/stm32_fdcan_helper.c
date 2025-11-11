/*
 * stm32_fdcan_helper.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */
#include "stm32_fdcan_helper.h"

/*
 * Function to parse the FDCAN_data_length_code present in the rx_header to an actual length value.
 * Returns zero on error
 * */
int get_stm32_fdcan_length(uint32_t code)
{
	uint32_t len_nibble =  (code >> 16) & 0xF;
	if(len_nibble >= 0 && len_nibble <= 8)
	{
		return (int)len_nibble;
	}
	else
	{
		if(code == FDCAN_DLC_CODE_12)
		{
			return 12;
		}
		else if(code == FDCAN_DLC_CODE_16)
		{
			return 16;
		}
		else if(code == FDCAN_DLC_CODE_20)
		{
			return 20;
		}
		else if(code == FDCAN_DLC_CODE_24)
		{
			return 24;
		}
		else if(code == FDCAN_DLC_CODE_32)
		{
			return 32;
		}
		else if(code == FDCAN_DLC_CODE_48)
		{
			return 48;
		}
		else if(code == FDCAN_DLC_CODE_64)
		{
			return 64;
		}
	}
	return 0;
}

uint32_t set_stm32_fdcan_code(int len)
{
	if(len > 0 && len <= 8)
	{
		return  (len & 0xF) << 16;
	}
	else if (len > 8)	//could build a function that uses division and modulo arithmetic to accomplish this but i believe this is more performant for short messages cus you fall thru the if statements
	{
		if(len == 12)
		{
			return  FDCAN_DLC_CODE_12;
		}
		else if(len == 16)
		{
			return  FDCAN_DLC_CODE_16;
		}
		else if(len == 20)
		{
			return  FDCAN_DLC_CODE_20;
		}
		else if(len == 24)
		{
			return  FDCAN_DLC_CODE_24;
		}
		else if(len == 32)
		{
			return  FDCAN_DLC_CODE_32;
		}
		else if(len == 48)
		{
			return  FDCAN_DLC_CODE_48;
		}
		else if(len == 64)
		{
			return  FDCAN_DLC_CODE_64;
		}
	}
	return 0;
}

