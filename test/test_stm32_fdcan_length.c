#include "stm32_fdcan_helper.h"
#include "unity.h"


void test_get(void)
{
    TEST_ASSERT_EQUAL(0, get_stm32_fdcan_length(FDCAN_DLC_CODE_0));
    TEST_ASSERT_EQUAL(1, get_stm32_fdcan_length(FDCAN_DLC_CODE_1));
    TEST_ASSERT_EQUAL(2, get_stm32_fdcan_length(FDCAN_DLC_CODE_2));
    TEST_ASSERT_EQUAL(3, get_stm32_fdcan_length(FDCAN_DLC_CODE_3));
    TEST_ASSERT_EQUAL(4, get_stm32_fdcan_length(FDCAN_DLC_CODE_4));
    TEST_ASSERT_EQUAL(5, get_stm32_fdcan_length(FDCAN_DLC_CODE_5));
    TEST_ASSERT_EQUAL(6, get_stm32_fdcan_length(FDCAN_DLC_CODE_6));
    TEST_ASSERT_EQUAL(7, get_stm32_fdcan_length(FDCAN_DLC_CODE_7));
    TEST_ASSERT_EQUAL(8, get_stm32_fdcan_length(FDCAN_DLC_CODE_8));
    TEST_ASSERT_EQUAL(12, get_stm32_fdcan_length(FDCAN_DLC_CODE_12));
    TEST_ASSERT_EQUAL(16, get_stm32_fdcan_length(FDCAN_DLC_CODE_16));
    TEST_ASSERT_EQUAL(20, get_stm32_fdcan_length(FDCAN_DLC_CODE_20));
    TEST_ASSERT_EQUAL(24, get_stm32_fdcan_length(FDCAN_DLC_CODE_24));
    TEST_ASSERT_EQUAL(32, get_stm32_fdcan_length(FDCAN_DLC_CODE_32));
    TEST_ASSERT_EQUAL(48, get_stm32_fdcan_length(FDCAN_DLC_CODE_48));
    TEST_ASSERT_EQUAL(64, get_stm32_fdcan_length(FDCAN_DLC_CODE_64));

    TEST_ASSERT_EQUAL(0, get_stm32_fdcan_length(0));
    TEST_ASSERT_EQUAL(0, get_stm32_fdcan_length(1));
    TEST_ASSERT_EQUAL(0, get_stm32_fdcan_length(0x8000));
    TEST_ASSERT_EQUAL(0, get_stm32_fdcan_length(65 << 24));
}

void test_set(void)
{
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_0, set_stm32_fdcan_code(0));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_1, set_stm32_fdcan_code(1));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_2, set_stm32_fdcan_code(2));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_3, set_stm32_fdcan_code(3));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_4, set_stm32_fdcan_code(4));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_5, set_stm32_fdcan_code(5));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_6, set_stm32_fdcan_code(6));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_7, set_stm32_fdcan_code(7));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_8, set_stm32_fdcan_code(8));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_12, set_stm32_fdcan_code(12));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_16, set_stm32_fdcan_code(16));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_20, set_stm32_fdcan_code(20));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_24, set_stm32_fdcan_code(24));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_32, set_stm32_fdcan_code(32));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_48, set_stm32_fdcan_code(48));
    TEST_ASSERT_EQUAL(FDCAN_DLC_CODE_64, set_stm32_fdcan_code(64));
    
    TEST_ASSERT_EQUAL(0, set_stm32_fdcan_code(63)); //error
    TEST_ASSERT_EQUAL(0, set_stm32_fdcan_code(128)); //error
    TEST_ASSERT_EQUAL(0, set_stm32_fdcan_code(17)); //error

}
