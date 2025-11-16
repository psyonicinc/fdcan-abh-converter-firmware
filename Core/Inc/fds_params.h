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
	uint32_t module_number;	//zero-pad this so it's 32bit aligned, for now
	uint32_t uart_baud_rate;
	uint32_t fdcan_NBRP;
	uint32_t fdcan_NTSEG1;
	uint32_t fdcan_NTSEG2;

}fds_params_t;

extern fds_params_t * p_fds_params;

#endif /* INC_FDS_PARAMS_H_ */
