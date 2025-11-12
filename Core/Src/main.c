#include "init.h"
#include "FDCAN.h"
#include "dartt_params.h"
#include "fds.h"
#include "m_dma_uart.h"


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



int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_FDCAN1_Init();
	MX_USART2_UART_Init();
	FDCAN_Config();

	load_flash_params(&fs_alias);
	unsigned char dartt_misc_address = dartt_get_complementary_address((unsigned char)dp.fds_p.module_number);

//	uint32_t can_cmd_exp_ts = 0;
	uint32_t led_ts = 0;
	uint8_t trigger_abh_write = 0;
	while (1)
	{
		uint32_t tick = HAL_GetTick();

		/*Handle fdcan access to dartt parameters*/
		if(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) != 0)
		{
			HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &can_rx_header, can_rx_data.u8);
			can_rx_alias.len = get_stm32_fdcan_length(can_rx_header.DataLength);
			if(can_rx_header.Identifier == dp.fds_p.module_number)	//motor command - dartt specifies custom implementation
			{
//				can_tx_header.Identifier = MASTER_MOTOR_ADDRESS;	//no dedicated 'motor messages' defined for this dartt device (yet)
//				HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_tx_header, can_tx_data.u8);
			}
			else if (can_rx_header.Identifier == dartt_misc_address)
			{
			    dartt_frame_to_payload(&can_rx_alias, TYPE_ADDR_CRC_MESSAGE, PAYLOAD_ALIAS, &gl_can_rx_pld_msg);
			    dartt_parse_general_message(&gl_can_rx_pld_msg, TYPE_ADDR_CRC_MESSAGE, &gl_dp_alias, &can_tx_alias);
			    if(can_tx_alias.len != 0)
			    {
			    	send_fdcan_frame(MASTER_MISC_ADDRESS, &can_tx_alias);
			    	trigger_abh_write = 1;
			    }
			}
		}


		if(trigger_abh_write)
		{
			switch(dp.abh_comms.command_header)
			{
				case FIXED_POSITION_CONTROL_TX1:
				case FIXED_POSITION_CONTROL_TX2:
				case FIXED_POSITION_CONTROL_TX3:
				case FIXED_VELOCITY_CONTROL_TX1:
				case FIXED_VELOCITY_CONTROL_TX2:
				case FIXED_VELOCITY_CONTROL_TX3:
				case FIXED_TORQUE_CONTROL_TX1:
				case FIXED_TORQUE_CONTROL_TX2:
				case FIXED_TORQUE_CONTROL_TX3:
				case FIXED_VOLTAGE_CONTROL_TX1:
				case FIXED_VOLTAGE_CONTROL_TX2:
				case FIXED_VOLTAGE_CONTROL_TX3:
				{
//					ahb_parse_movement_reply(&m_huart2.rx_decode_alias, &dp.abh_comms);
					abh_create_movement_frame(&dp.abh_comms, &abh_cmd_alias);
					abh_ppp_unstuffed_cmd_alias.length = abh_cmd_alias.len;	//type translation - point to same buffer but length must be copied through. a bit inelegant
					PPP_stuff(&abh_ppp_unstuffed_cmd_alias, &m_huart2.tx_mem);
					m_uart_dma_transmit(&m_huart2);	//this function uses encoded length, so a second length copy is not necessary
					break;
				}
				case FIXED_DUMMY_TX1:
				case FIXED_DUMMY_TX2:
				case FIXED_DUMMY_TX3:
				case ENABLE_BLUETOOTH_RADIO:
				case DISABLE_BLUETOOTH_RADIO:
				case RESTART_HAND:
				case API_EXIT_CMD:
				{
					abh_create_short_api_frame(&dp.abh_comms, &abh_cmd_alias);
					abh_ppp_unstuffed_cmd_alias.length = abh_cmd_alias.len;	//type translation - point to same buffer but length must be copied through. a bit inelegant
					PPP_stuff(&abh_ppp_unstuffed_cmd_alias, &m_huart2.tx_mem);
					m_uart_dma_transmit(&m_huart2);	//this function uses encoded length, so a second length copy is not necessary
					break;
				}
				case UART_WRITE_REGISTER:
				{
					break;
				}
				case UART_READ_REGISTER:
				{
					break;
				}
				//todo: handle read/write register cases
				default:
				{
					return ABH_ERROR_INVALID_HEADER;
				}

			};

			trigger_abh_write = 0;
		}





		/*Reply Parser*/
		if(m_huart2.rx_decoded.length != 0)	//
		{
			m_huart2.rx_decoded.length = 0;
			switch(dp.abh_comms.command_header)
			{
				case FIXED_DUMMY_TX1:
				case FIXED_DUMMY_TX2:
				case FIXED_DUMMY_TX3:
				case FIXED_POSITION_CONTROL_TX1:
				case FIXED_POSITION_CONTROL_TX2:
				case FIXED_POSITION_CONTROL_TX3:
				case FIXED_VELOCITY_CONTROL_TX1:
				case FIXED_VELOCITY_CONTROL_TX2:
				case FIXED_VELOCITY_CONTROL_TX3:
				case FIXED_TORQUE_CONTROL_TX1:
				case FIXED_TORQUE_CONTROL_TX2:
				case FIXED_TORQUE_CONTROL_TX3:
				case FIXED_VOLTAGE_CONTROL_TX1:
				case FIXED_VOLTAGE_CONTROL_TX2:
				case FIXED_VOLTAGE_CONTROL_TX3:
				{
					ahb_parse_movement_reply(&m_huart2.rx_decode_alias, &dp.abh_comms);
					break;
				}
				case UART_WRITE_REGISTER:
				{
					break;
				}
				case UART_READ_REGISTER:
				{
					break;
				}
				//todo: handle read/write register cases
				default:
				{
					return ABH_ERROR_INVALID_HEADER;
				}
			};
		}


		if(tick-led_ts > 500)
		{
			HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
			led_ts = tick;
		}

	}
}
