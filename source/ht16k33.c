/*
 * ht16k33.c
 *
 *  Created on: Dec 15, 2018
 *      Author: ben
 */

#include <ht16k33.h>

#define REFRESH_BIT 0x01
#define TX_BIT 0x02
#define HT16K33_STACK_SIZE configMINIMAL_STACK_SIZE

static StaticTask_t ht16k33TaskTCBBuffer;
static StackType_t ht16k33TaskStack[HT16K33_STACK_SIZE];
static EventGroupHandle_t ht16k33EventGroupHandle;
static HT16K33_t ht16k33s[FREQUENCIES];
static TaskHandle_t ht16k33TaskHandle;

void HT16K33_SetBarColor(HT16K33_t *ht16k33, uint8_t bar, Bar_Color_t color);
void HT16K33_SetLevel(HT16K33_t *ht16k33, uint32_t level);
void HT16K33_Task(void *pvParameters);
void HT16K33_TransferCallback(I2C_Type *base, i2c_master_handle_t *handle,
		status_t status, void *userData);

void HT16K33_Init(void) {
	uint32_t i;
	HT16K33_t *ht16k33;
	ftm_config_t ftm0_config;

	uint8_t commandBuffer[3] = { HT16K33_SYSTEM_SETUP | 0x01, // Turn on the system oscillator
	HT16K33_DISPLAY_SETUP | 0x01,	// Display on
			};

	ht16k33EventGroupHandle = xEventGroupCreate();
	configASSERT(ht16k33EventGroupHandle);

	//TODO write out zeros to clear the display bits
	for (i = 0; i < FREQUENCIES; i++) {
		ht16k33 = &ht16k33s[i];
		ht16k33->address = 0x70 + i;

		I2C_1_transfer.slaveAddress = ht16k33->address;
		I2C_1_transfer.dataSize = 0x01;

		I2C_1_transfer.data = &commandBuffer[0];
		I2C_MasterTransferBlocking(I2C_1_PERIPHERAL, &I2C_1_transfer);
		I2C_1_transfer.data = &commandBuffer[1];
		I2C_MasterTransferBlocking(I2C_1_PERIPHERAL, &I2C_1_transfer);
	}

	ht16k33TaskHandle = xTaskCreateStatic(HT16K33_Task, "HT16K33", HT16K33_STACK_SIZE, NULL,
	configMAX_PRIORITIES - 1, ht16k33TaskStack, &ht16k33TaskTCBBuffer);

	/**
	 * The FTM counter is 16 bit.
	 * Therefore, the counter value must be below 0xFFFF
	 * The "system" clock (actually the bus clock) runs at 60 MHz
	 * 1,000,000 cycles at 60 MHz are required to hit 60Hz
	 * 1,000,000 / 16 = 62500 which is below 0xFFFF
	 */
	FTM_GetDefaultConfig(&ftm0_config);
	ftm0_config.prescale = kFTM_Prescale_Divide_16;
	FTM_Init(FTM0, &ftm0_config);
	FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
	FTM_SetTimerPeriod(FTM0, 30000);
	NVIC_SetPriority(FTM0_IRQn, 4);
	EnableIRQ(FTM0_IRQn);
	FTM_StartTimer(FTM0, kFTM_SystemClock);

	NVIC_SetPriority(I2C0_IRQn, 4);
}

void HT16K33_Task(void *pvParameters) {
	HT16K33_t *ht16k33;
	static uint32_t i;
	static uint32_t level;
	static uint8_t writeBuffer[0x07];

	/**
	 * All I2C transfers will be of the same length and use writeBuffer
	 */
	writeBuffer[0] = HT16K33_DISPLAY_DATA_ADDRESS;
	I2C_1_transfer.data = &writeBuffer[0];
	I2C_1_transfer.dataSize = 0x07;

	for (;;) {
		/**
		 * Wait for the refresh rate event to come around (60Hz)
		 */
		xEventGroupWaitBits(ht16k33EventGroupHandle, REFRESH_BIT, pdTRUE, pdTRUE,
		portMAX_DELAY);

		for (i = 0; i < FREQUENCIES; i++) {
			level = msgeq7Results[i];
			/**
			 * Cut the noise floor
			 */
			if (level > 0x18) {
				level -= 0x18;
			} else {
				level = 0;
			}

			level = (level * 23) / 0xE7;

			ht16k33 = &ht16k33s[i];
			if (level == ht16k33->current_level) {
				continue;
			}

			HT16K33_SetLevel(ht16k33, level);

			/**
			 * Setup and start the I2C transfer
			 */
			memcpy(&writeBuffer[1], ht16k33->display_buffer, 0x06);
			I2C_1_transfer.slaveAddress = ht16k33->address;
			I2C_MasterTransferNonBlocking(I2C_1_PERIPHERAL, &I2C_1_handle,
					&I2C_1_transfer);
			/**
			 * Wait for the transfer to complete before going onto the next one
			 */
			xEventGroupWaitBits(ht16k33EventGroupHandle, TX_BIT, pdTRUE,
					pdTRUE,
					portMAX_DELAY);
		}
	}
}

void HT16K33_SetLevel(HT16K33_t *ht16k33, uint32_t level) {
	uint32_t i = 0;

	if (ht16k33->current_level < level) {
		for (i = ht16k33->current_level; i <= level; i++) {
			if (i < 15) {
				HT16K33_SetBarColor(ht16k33, i, BAR_GREEN);
			} else if (i < 21) {
				HT16K33_SetBarColor(ht16k33, i, BAR_YELLOW);
			} else {
				HT16K33_SetBarColor(ht16k33, i, BAR_RED);
			}
		}

		ht16k33->current_level = level;
	} else {
		HT16K33_SetBarColor(ht16k33, ht16k33->current_level, BAR_OFF);
		ht16k33->current_level--;
	}

}

/* The low nibble is one LED Bar Graph
 * and the high nibble is the other
 * RED,	GREEN
 * 0x00, 0x00,	LED 1-4
 * 0x00, 0x00,	LED 5-8
 * 0x00, 0x00,	LED 9-12
 */
void HT16K33_SetBarColor(HT16K33_t *ht16k33, uint8_t bar, Bar_Color_t color) {
	uint8_t index, graph, shift;

	if (bar >= 12) {
		bar -= 12;
		graph = 4;
	} else {
		graph = 0;
	}

	if (bar < 4) {
		index = 0;
	} else if (bar < 8) {
		bar -= 4;
		index = 2;
	} else if (bar < 12) {
		bar -= 8;
		index = 4;
	} else {
		return;
	}

	shift = (bar + graph);

	switch (color) {
	case BAR_OFF:
		ht16k33->display_buffer[index] &= ~(0x01 << shift);
		ht16k33->display_buffer[index + 1] &= ~(0x01 << shift);
		break;
	case BAR_RED:
		ht16k33->display_buffer[index] |= 0x01 << shift;
		ht16k33->display_buffer[index + 1] &= ~(0x01 << shift);
		break;
	case BAR_GREEN:
		ht16k33->display_buffer[index] &= ~(0x01 << shift);
		ht16k33->display_buffer[index + 1] |= 0x01 << shift;
		break;
	case BAR_YELLOW:
		ht16k33->display_buffer[index] |= 0x01 << shift;
		ht16k33->display_buffer[index + 1] |= 0x01 << shift;
		break;
	}
}

/**
 * A callback for the non-blocking I2C transfers
 */
void DISPLAY_TransferCallback(I2C_Type *base, i2c_master_handle_t *handle,
		status_t status, void *userData) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static BaseType_t xResult;

	I2C_MasterClearStatusFlags(I2C_1_PERIPHERAL, kI2C_IntPendingFlag);
	xResult = xEventGroupSetBitsFromISR(ht16k33EventGroupHandle, TX_BIT,
			&xHigherPriorityTaskWoken);

	if (xResult != pdFAIL) {		//was the message posted successfully?
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

/**
 * clear the interrupt flag
 * send the refresh bits to the main task
 */
void FTM0_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static BaseType_t xResult;

	FTM0->SC &= ~FTM_SC_TOF_MASK;

	xResult = xEventGroupSetBitsFromISR(ht16k33EventGroupHandle, REFRESH_BIT,
			&xHigherPriorityTaskWoken);

	if (xResult != pdFAIL) {		//was the message posted successfully?
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
