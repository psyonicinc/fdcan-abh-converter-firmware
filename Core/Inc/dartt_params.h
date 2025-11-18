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
#include "m_dma_uart.h"

#define UART_RX_RECV_SIZE 	152
#define UART_IT_BUF_SIZE 	76		//fw generically capable of handling 24 bytes incoming.

#define NBYTES_GIT_HASH_BUFFER	 16

typedef struct dartt_params_t
{
	//ability hand struct
	fds_params_t fds_p;
	abh_api_t abh_comms;

	uint32_t register_target;	//target ability hand register
	uint32_t register_write_val;	//write value to the register
	uint32_t register_read_reply_val;	//read value from the register
	uint32_t abh_read_timeout;	//timeout for awaiting a read reply

	uint8_t uart_rx_decoded[UART_IT_BUF_SIZE];	//buffer containing raw (HDLC decoded) data frames, recieved over UART from the interface controller.
	uint32_t nbytes_decoded_uart;	//number of bytes in the decoded buffer. Defines the valid buffer size.
	uint8_t uart_tx_mem[UART_IT_BUF_SIZE];	//pad this by 2 bytes so the struct stays 32bit aligned
	uint32_t nbytes_write_uart;			//number of bytes to write over UART. Updating this value triggers a write

	uint8_t git_hash_buffer[NBYTES_GIT_HASH_BUFFER];

	uint8_t update_nonvolatile_storage;	//flag which, when set, triggers a one-time filesystem update. Flag is cleared upon completion of update operation.

}dartt_params_t;

extern dartt_params_t dp;
extern buffer_t fs_alias;

#endif /* INC_DARTT_PARAMS_H_ */
