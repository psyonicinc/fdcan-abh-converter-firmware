/*
 * dartt_params.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */
#include "dartt_params.h"


dartt_params_t dp =
{
		.fds_p = {
				.module_number = 0x50
		}
};

fds_params_t * p_fds_params = &dp.fds_p;	//shortcut to filesystem parameters
