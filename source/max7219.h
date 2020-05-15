/*
 * max7219.h
 *
 *  Created on: Jan 4, 2019
 *      Author: ben
 */

#include <rotary_encoder.h>
#include <stdlib.h>
#include "fsl_dspi.h"
#include "peripherals.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#ifndef MAX7219_H_
#define MAX7219_H_

typedef enum {
	MAX7219_CodeB_0,
	MAX7219_CodeB_1,
	MAX7219_CodeB_2,
	MAX7219_CodeB_3,
	MAX7219_CodeB_4,
	MAX7219_CodeB_5,
	MAX7219_CodeB_6,
	MAX7219_CodeB_7,
	MAX7219_CodeB_8,
	MAX7219_CodeB_9,
	MAX7219_CodeB_Minus,
	MAX7219_CodeB_H,
	MAX7219_CodeB_E,
	MAX7219_CodeB_L,
	MAX7219_CodeB_P,
	MAX7219_CodeB_Blank

} MAX7219_CodeB_t;

typedef enum {
	MAX7219_Address_NoOp,
	MAX7219_Address_Digit0,
	MAX7219_Address_Digit1,
	MAX7219_Address_Digit2,
	MAX7219_Address_Digit3,
	MAX7219_Address_Digit4,
	MAX7219_Address_Digit5,
	MAX7219_Address_Digit6,
	MAX7219_Address_Digit7,
	MAX7219_Address_DecodeMode,
	MAX7219_Address_Intensity,
	MAX7219_Address_ScanLimit,
	MAX7219_Address_Shutdown,
	MAX7219_Address_DisplayTest,
} MAX7219_Address_t;

extern EventGroupHandle_t segmentEventGroupHandle;

void MAX7219_Init(void);

#endif /* MAX7219_H_ */
