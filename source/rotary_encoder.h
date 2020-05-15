/*
 * rotary_encoder.h
 *
 *  Created on: Jan 3, 2019
 *      Author: ben
 */

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

#include "pin_mux.h"
#include "board.h"
#include "fsl_gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "event_groups.h"

#define ROTARY_ENCODER_LEFT 0x01
#define ROTARY_ENCODER_RIGHT 0x02
#define ROTARY_ENCODER_CLICK 0x04
#define MAX_VOLUME 6
#define MIN_VOLUME -59

extern EventGroupHandle_t rotaryEncoderEventGroupHandle;
extern volatile int currentVolume;

void ROTARY_ENCODER_Init(void);

#endif /* ROTARY_ENCODER_H_ */
