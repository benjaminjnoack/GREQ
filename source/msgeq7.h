/*
 * msgeq7.h
 *
 *  Created on: Dec 19, 2018
 *      Author: ben
 */

#ifndef MSGEQ7_H_
#define MSGEQ7_H_

#include "pin_mux.h"
#include "peripherals.h"
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "fsl_adc16.h"
#include "fsl_ftm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "frequency.h"

#define COUNT_32US_AT_60MHz 1920
#define COUNT_72US_AT_60MHz 4320
#define NUM_SAMPLES 0x0A

extern uint32_t msgeq7Results[FREQUENCIES];

void MSGEQ7_Init(void);

#endif /* MSGEQ7_H_ */
