/*
 * dartt_params.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */

#ifndef INC_DARTT_PARAMS_H_
#define INC_DARTT_PARAMS_H_
#include "fds_params.h"

typedef struct dartt_params_t
{
	//ability hand struct
	fds_params_t fds_p;
}dartt_params_t;

extern dartt_params_t dartt_params;

#endif /* INC_DARTT_PARAMS_H_ */
