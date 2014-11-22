/*
 * RTCSimple.c
 *
 *  Created on: Jul 29, 2014
 *      Author: fsoo
 */

#include "RTCSimple.h"


PE_ISR(RTCInterrrupt)
{
// NOTE: The routine should include actions to clear the appropriate
//       interrupt flags.
//
}
void enableRTC()
{
	
	RTC_PDD_WriteControlReg(RTC_BASE_PTR,0x1900); // enable oscillator

//	vTaskDelay(10);
	RTC_PDD_EnableCounter(RTC_BASE_PTR, PDD_ENABLE);

	
};

uint32 readRTC()
{
	return RTC_PDD_ReadTimeSecondsReg(RTC_BASE_PTR);
	
}
