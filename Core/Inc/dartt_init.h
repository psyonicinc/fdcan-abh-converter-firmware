/*
 * dartt_init.h
 *
 *  Created on: Nov 14, 2025
 *      Author: ocanath
 */

#ifndef INC_DARTT_INIT_H_
#define INC_DARTT_INIT_H_

#include "dartt.h"
#include "PPP.h"

extern payload_layer_msg_t gl_can_rx_pld_msg;
extern buffer_t can_rx_alias;
extern buffer_t can_tx_alias;
extern buffer_t gl_dp_alias;
extern buffer_t abh_cmd_alias;
extern ppp_buffer_t abh_ppp_unstuffed_cmd_alias;


#endif /* INC_DARTT_INIT_H_ */
