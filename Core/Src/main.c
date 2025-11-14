#include "init.h"
#include "FDCAN.h"
#include "dartt_params.h"
#include "fds.h"
#include "m_dma_uart.h"
#include "dartt_init.h"


typedef struct abh_read_reply_flags_t
{
	uint8_t read_pending;
	uint32_t read_ts;
}abh_read_reply_flags_t;

abh_read_reply_flags_t gl_rrep = {};

int flag_read_reply(abh_read_reply_flags_t * f, uint32_t tick)
{
	if(f == NULL)
	{
		return ERROR_INVALID_ARGUMENT;
	}
	f->read_pending = 1;
	f->read_ts = tick;
	return 0;
}

static uint8_t trigger_abh_write = 0;	//global for live expressions access

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_FDCAN1_Init();
	FDCAN_Config();

	load_flash_params(&fs_alias);
	unsigned char dartt_misc_address = dartt_get_complementary_address((unsigned char)dp.fds_p.module_number);
	MX_USART2_UART_Init();

//	uint32_t can_cmd_exp_ts = 0;
	uint32_t led_ts = 0;
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
			    trigger_abh_write = 1;
			    if(can_tx_alias.len != 0)
			    {
			    	send_fdcan_frame(MASTER_MISC_ADDRESS, &can_tx_alias);
			    }
			}
		}


		if(trigger_abh_write && gl_rrep.read_pending == 0)	//don't retransmit while awaiting an ability hand reply frame
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
					abh_create_movement_frame(&dp.abh_comms, &abh_cmd_alias);
					abh_ppp_unstuffed_cmd_alias.length = abh_cmd_alias.len;	//type translation - point to same buffer but length must be copied through. a bit inelegant
					PPP_stuff(&abh_ppp_unstuffed_cmd_alias, &m_huart2.tx_mem);
					m_uart_dma_transmit(&m_huart2);	//this function uses encoded length, so a second length copy is not necessary
					flag_read_reply(&gl_rrep, tick);
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
					flag_read_reply(&gl_rrep, tick);
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
				//TODO: handle read/write register cases
				default:	//if the command header is invalid (i.e. zero) and we have a nonzero number of requested bytes to write via dartt, write them out!
				{
					if(dp.nbytes_write_uart != 0)
					{
						//queue out a transmission
						m_huart2.tx_mem.length = dp.nbytes_write_uart;
						dp.nbytes_write_uart = 0;	//clear to prevent repeat transmissions
						m_uart_dma_transmit(&m_huart2);	//this function uses encoded length, so a second length copy is not necessary
					}
					break;
				}

			};
			trigger_abh_write = 0;

			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, 1);
			led_ts = tick;
		}

		if(gl_rrep.read_pending != 0)
		{
			if((tick - gl_rrep.read_ts) > dp.abh_read_timeout)
			{
				gl_rrep.read_pending = 0;	//time out
			}
		}


		/*Reply Parser*/
		if(m_huart2.rx_decoded.length != 0)	//race condition if the command header is overwritten during the subsequent uart command exchange. proper logic should copy the command header  to local var on a write initialization, then
		{
			gl_rrep.read_pending = 0;
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
				//TODO: handle read/write register cases
				default:	//do something on error
				{
					break;	// ABH_ERROR_INVALID_HEADER;
				}
			};
		}


		if(dp.update_nonvolatile_storage != 0)
		{
			m_write_flash((uint64_t*)fs_alias.buf,fs_alias.size/sizeof(uint64_t));
			dp.update_nonvolatile_storage = 0;
		}

		if(tick-led_ts > 25)
		{
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, 0);
		}

	}
}
