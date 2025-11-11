/*
 * dartt_params.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */
#include "dartt_params.h"


dartt_params_t dp =
{
		.abh_comms = {
				.address = 0x50,
				.command_header = FIXED_DUMMY_TX1
		},
		.fds_p = {
				.module_number = 0x50,
				.uart_baud_rate = 460800,
				.can_baud_rate = 2125000
		}
};

fds_params_t * p_fds_params = &dp.fds_p;	//shortcut to filesystem parameters


buffer_t fs_alias = {
		.buf = (unsigned char *)(&dp.fds_p),
		.size = sizeof(fds_params_t),
		.len = sizeof(fds_params_t)
};
