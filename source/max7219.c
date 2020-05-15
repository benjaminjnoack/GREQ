/*
 * segment.c
 *
 *  Created on: Jan 4, 2019
 *      Author: ben
 */

#include <max7219.h>

#define MAX7219_STACK_SIZE configMINIMAL_STACK_SIZE

static StaticTask_t segmentTaskTCBBuffer;
static StackType_t segmentTaskStack[MAX7219_STACK_SIZE];
static TaskHandle_t segmentTaskHandle;
static dspi_transfer_t transferHandle;

uint8_t displayBuffer[3][2] = { { MAX7219_Address_Digit0, MAX7219_CodeB_Blank },
		{ MAX7219_Address_Digit1, MAX7219_CodeB_0 }, { MAX7219_Address_Digit2,
				MAX7219_CodeB_0 } };
EventGroupHandle_t segmentEventGroupHandle;

void MAX7219_Task(void *pvParameters);

void MAX7219_Init(void) {
	status_t status;

	static uint8_t initCommands[7][2] = { { MAX7219_Address_Shutdown, 0x01 }, //Normal Operation
			{ MAX7219_Address_ScanLimit, 0x02 },	//Display Digits 0,1,2
			{ MAX7219_Address_Intensity, 0x07 },	//Max Brightness
			{ MAX7219_Address_DecodeMode, 0x07 },	//Code B Decode for Digits
			{ MAX7219_Address_Digit0, MAX7219_CodeB_Blank },//Initial Digit Value
			{ MAX7219_Address_Digit1, MAX7219_CodeB_0 },//Initial Digit Value
			{ MAX7219_Address_Digit2, MAX7219_CodeB_0 }	//Initial Digit Value
	};

	transferHandle.rxData = DSPI_1_rxBuffer;
	transferHandle.txData = DSPI_1_txBuffer;
	transferHandle.dataSize = sizeof(DSPI_1_txBuffer);
	transferHandle.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0
			| kDSPI_MasterPcsContinuous;

	for (uint32_t i = 0; i < 7; i++) {
		transferHandle.txData[0] = initCommands[i][0];
		transferHandle.txData[1] = initCommands[i][1];
		status = DSPI_MasterTransferBlocking(DSPI_1_PERIPHERAL,
				&transferHandle);
		if (status != kStatus_Success) {
			while (1)
				;
		}
	}

	segmentEventGroupHandle = xEventGroupCreate();
	configASSERT(segmentEventGroupHandle);

	segmentTaskHandle = xTaskCreateStatic(MAX7219_Task, "MAX7219",
	MAX7219_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, segmentTaskStack,
			&segmentTaskTCBBuffer);
}

void MAX7219_Task(void *pvParameters) {
	static EventBits_t bits;
	static status_t status;
	static int displayValue;
	static uint8_t buf[3] = { 0x00, 0x00, 0x00 };

	for (;;) {
		bits = xEventGroupWaitBits(rotaryEncoderEventGroupHandle,
				(ROTARY_ENCODER_LEFT | ROTARY_ENCODER_RIGHT | ROTARY_ENCODER_CLICK), pdTRUE, pdFALSE,
				portMAX_DELAY);

		if (bits == ROTARY_ENCODER_CLICK) {
			continue;
		}

		displayValue = currentVolume;

		if (displayValue < 0) {
			displayValue = abs(displayValue);
			buf[0] = MAX7219_CodeB_Minus;
		} else {
			buf[0] = MAX7219_CodeB_Blank;
		}

		buf[1] = displayValue / 10;
		buf[2] = displayValue % 10;

		for (uint32_t i = 0; i < 3; i++) {
			if (displayBuffer[i][1] == buf[i]) {
				continue;
			}

			transferHandle.txData[0] = displayBuffer[i][0];
			transferHandle.txData[1] = buf[i];
			status = DSPI_MasterTransferBlocking(DSPI_1_PERIPHERAL,
					&transferHandle);
			if (status != kStatus_Success) {
				while (1)
					;
			}

			displayBuffer[i][1] = buf[i];
		}
	}
}

void MAX7219_Callback(SPI_Type *base, dspi_master_handle_t *handle,
		status_t status, void *userData) {
	static BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = xTaskNotifyFromISR(segmentTaskHandle, 0x00,
			eNoAction, &xHigherPriorityTaskWoken);
}
