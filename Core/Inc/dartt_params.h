/*
 * dartt_params.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */

#ifndef INC_DARTT_PARAMS_H_
#define INC_DARTT_PARAMS_H_
#include "fds_params.h"
#include "abh_communication.h"

typedef struct dartt_params_t
{
	//ability hand struct
	fds_params_t fds_p;
	abh_api_t abh_comms;

	uint32_t register_target;	//target ability hand register
	uint32_t register_write_val;	//write value to the register
	uint32_t register_read_reply_val;	//read value from the register
}dartt_params_t;

extern dartt_params_t dp;
extern buffer_t fs_alias;

#endif /* INC_DARTT_PARAMS_H_ */
