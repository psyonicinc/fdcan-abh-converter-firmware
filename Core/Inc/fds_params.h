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

}fds_params_t;

extern fds_params_t * p_fds_params;

#endif /* INC_FDS_PARAMS_H_ */
