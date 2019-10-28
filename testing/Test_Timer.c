/*
 * Test_Timer.c
 *
 *  Created on: Jun 19, 2019
 *      Author: kenan
 */


#include "unity.h"
#include "../include/Timer.h"


void erster_Test(void)
{
	int TCNT1 = 9999;
	reset_and_start_timer();
	TEST_ASSERT_EQUAL_UINT16(0x0000,TCNT1);
}

int main(void){
UNITY_BEGIN();
RUN_TEST(erster_Test);
return UNITY_END();
}
