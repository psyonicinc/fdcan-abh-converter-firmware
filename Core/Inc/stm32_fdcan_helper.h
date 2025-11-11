/*
 * stm32_fdcan_helper.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Ocanath Robotman
 */

#ifndef INC_STM32_FDCAN_HELPER_H_
#define INC_STM32_FDCAN_HELPER_H_
#include <stdint.h>


#define FDCAN_DLC_CODE_0  ((uint32_t)0x00000000U) /*!< 0 bytes data field  */
#define FDCAN_DLC_CODE_1  ((uint32_t)0x00010000U) /*!< 1 bytes data field  */
#define FDCAN_DLC_CODE_2  ((uint32_t)0x00020000U) /*!< 2 bytes data field  */
#define FDCAN_DLC_CODE_3  ((uint32_t)0x00030000U) /*!< 3 bytes data field  */
#define FDCAN_DLC_CODE_4  ((uint32_t)0x00040000U) /*!< 4 bytes data field  */
#define FDCAN_DLC_CODE_5  ((uint32_t)0x00050000U) /*!< 5 bytes data field  */
#define FDCAN_DLC_CODE_6  ((uint32_t)0x00060000U) /*!< 6 bytes data field  */
#define FDCAN_DLC_CODE_7  ((uint32_t)0x00070000U) /*!< 7 bytes data field  */
#define FDCAN_DLC_CODE_8  ((uint32_t)0x00080000U) /*!< 8 bytes data field  */
#define FDCAN_DLC_CODE_12 ((uint32_t)0x00090000U) /*!< 12 bytes data field */
#define FDCAN_DLC_CODE_16 ((uint32_t)0x000A0000U) /*!< 16 bytes data field */
#define FDCAN_DLC_CODE_20 ((uint32_t)0x000B0000U) /*!< 20 bytes data field */
#define FDCAN_DLC_CODE_24 ((uint32_t)0x000C0000U) /*!< 24 bytes data field */
#define FDCAN_DLC_CODE_32 ((uint32_t)0x000D0000U) /*!< 32 bytes data field */
#define FDCAN_DLC_CODE_48 ((uint32_t)0x000E0000U) /*!< 48 bytes data field */
#define FDCAN_DLC_CODE_64 ((uint32_t)0x000F0000U) /*!< 64 bytes data field */


int get_stm32_fdcan_length(uint32_t code);
uint32_t set_stm32_fdcan_code(int len);

#endif /* INC_STM32_FDCAN_HELPER_H_ */
