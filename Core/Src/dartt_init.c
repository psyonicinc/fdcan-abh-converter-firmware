/*
 * dartt_init.c
 *
 *  Created on: Nov 14, 2025
 *      Author: ocanath
 */
#include "dartt_init.h"
#include "FDCAN.h"
#include "version.h"


//dartt rx payload helper memory
payload_layer_msg_t gl_can_rx_pld_msg = {};

//rx alias buffer
buffer_t can_rx_alias =
{
		.buf = &can_rx_data.u8[0],
		.size = sizeof(can_rx_data),
		.len = 0
};

//tx alias buffer
buffer_t can_tx_alias =
{
		.buf = &can_tx_data.u8[0],
		.size = sizeof(can_rx_data),
		.len = 0
};

//dartt params buffer alias
buffer_t gl_dp_alias =
{
		.buf = (unsigned char *)(&dp),
		.size = sizeof(dp),
		.len = 0
};

unsigned char cmd_buf[32] = {};	//32 is oversized - only needs to be 15
buffer_t abh_cmd_alias =
{
		.buf = cmd_buf,
		.size = sizeof(cmd_buf),
		.len = 0
};
ppp_buffer_t abh_ppp_unstuffed_cmd_alias =
{
		.buf = cmd_buf,
		.size = sizeof(cmd_buf),
		.length = 0
};


void copy_git_hash_to_dartt(dartt_params_t * p_dp)
{
	for(int i = 0; firmware_version[i] != 0 && i < sizeof(p_dp->git_hash_buffer); i++)
	{
		p_dp->git_hash_buffer[i] = firmware_version[i];
	}
}
