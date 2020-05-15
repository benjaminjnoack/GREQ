/*
 * potentiometer.h
 *
 *  Created on: Dec 29, 2018
 *      Author: ben
 */

#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_adc16.h"
#include "fsl_ftm.h"
#include "fsl_gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "event_groups.h"

extern uint32_t equalizerResults[5];

void POTENTIOMETER_Init(void);

#endif /* POTENTIOMETER_H_ */
