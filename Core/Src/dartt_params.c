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
				.module_number = 0x50,
				.uart_baud_rate = 460800,
				.fdcan_NBRP = 2,
				.fdcan_NTSEG1 = 135,
				.fdcan_NTSEG2 = 34
		},
		.abh_comms = {
				.address = 0x50,
				.command_header = FIXED_DUMMY_TX1
		},

		.register_target = 0,
		.register_write_val = 0,
		.register_read_reply_val = 0,
		.abh_read_timeout = 5,

		.uart_rx_decoded = {},
		.nbytes_decoded_uart = 0,
		.uart_tx_mem = {},
		.nbytes_write_uart = 0,

		.git_hash_buffer = {},

		.update_nonvolatile_storage = 0
};

fds_params_t * p_fds_params = &dp.fds_p;	//shortcut to filesystem parameters


buffer_t fs_alias = {
		.buf = (unsigned char *)(&dp.fds_p),
		.size = sizeof(fds_params_t),
		.len = sizeof(fds_params_t)
};
