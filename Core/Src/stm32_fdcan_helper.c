/*
 * stm32_fdcan_helper.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */
#include "stm32_fdcan_helper.h"

//lut for get length.
static uint8_t fdcan_length_lookup[] =
{
		0,
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		12,
		16,
		20,
		24,
		32,
		48,
		64
};


/*
 * Function to parse the FDCAN_data_length_code present in the rx_header to an actual length value.
 * Returns zero on error
 * */
int get_stm32_fdcan_length(uint32_t code)
{
	uint32_t len_nibble =  (code >> 16) & 0xF;
	if(len_nibble >= 0 && len_nibble < sizeof(fdcan_length_lookup)/sizeof(uint8_t))
	{
		return (int)fdcan_length_lookup[len_nibble];
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

