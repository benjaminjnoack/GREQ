/*
 * ht16k33.h
 *
 *  Created on: Dec 15, 2018
 *      Author: ben
 *
 */

#ifndef HT16K33_H_
#define HT16K33_H_

#include <msgeq7.h>
#include <potentiometer.h>
#include <stdint.h>
#include "frequency.h"
#include "peripherals.h"
#include "fsl_i2c.h"
#include "fsl_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

typedef enum {
	BAR_OFF,
	BAR_RED,
	BAR_GREEN,
	BAR_YELLOW
} Bar_Color_t;

typedef enum {
	HT16K33_DISPLAY_DATA_ADDRESS,
	HT16K33_SYSTEM_SETUP = 0x20,
	HT16K33_KEY_DATA_ADDRESS = 0x40,
	HT16K33_INT_FLAG_ADDRESS = 0x60,
	HT16K33_DISPLAY_SETUP = 0x80,
	HT16K33_ROW_INT_SET = 0xA0,
	HT16K33_DIMMING_SET = 0xE0
} HT16K33_Command_t;

typedef struct {
	uint8_t address;
	uint8_t current_level;
	uint8_t display_buffer[6];
} HT16K33_t;

void HT16K33_Init(void);

#endif /* HT16K33_H_ */
