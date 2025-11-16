/*
 * fds_params.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */

#ifndef INC_FDS_PARAMS_H_
#define INC_FDS_PARAMS_H_
#include <stdint.h>

typedef struct fds_params_t
{
	uint32_t module_number;
	uint32_t uart_baud_rate;
	uint32_t fdcan_NBRP;
	uint32_t fdcan_NTSEG1;
	uint32_t fdcan_NTSEG2;
	uint32_t unused_zeropad;	//fds structures on this processor must be 64-bit aligned or else the last value will get truncated due to integer  division underflow

}fds_params_t;

extern fds_params_t * p_fds_params;

#endif /* INC_FDS_PARAMS_H_ */
