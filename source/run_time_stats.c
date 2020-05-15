/*
 * run_time_stats.c
 *
 *  Created on: Jan 16, 2019
 *      Author: ben
 */

#include "run_time_stats.h"

void vConfigureTimerForRunTimeStats(void)
{
	ftm_config_t ftm3_config;
	/**
	 * The target frequency is 10 to 100 times higher than the tick interrupt
	 * SysTick reloads at 899,999
	 * The system clock (180 MHz) divided by the reload yields 200 Hz
	 * Therefore, the goal is 2kHz to 20kHz
	 * The FTM Fixed Clock is the MCGFFCLK (375kHz)
	 * Divided by 128 yields 2.929 kHz
	 */
	FTM_GetDefaultConfig(&ftm3_config);
	ftm3_config.prescale = kFTM_Prescale_Divide_128;
	FTM_Init(FTM3, &ftm3_config);
	FTM_SetTimerPeriod(FTM3, 0xFFFF);
	FTM_StartTimer(FTM3, kFTM_FixedClock);
}
