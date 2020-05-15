/*
 * spectrum.c
 *
 *  Created on: Dec 20, 2018
 *      Author: ben
 */

#include <msgeq7.h>

#define MSGEQ7_STACK_SIZE configMINIMAL_STACK_SIZE

static StaticTask_t msgeq7TaskTCBBuffer;
static StackType_t msgeq7TaskStack[MSGEQ7_STACK_SIZE];
static TaskHandle_t msgeq7TaskHandle;

volatile uint32_t adcConversionResult = 0;
uint32_t msgeq7Results[FREQUENCIES];
static uint32_t samples[FREQUENCIES][NUM_SAMPLES];
static uint32_t *pSample[FREQUENCIES];

void MSGEQ7_Task(void *pvParameters);

void MSGEQ7_Init(void) {
	ftm_config_t ftm1_config;
	int i;

	msgeq7TaskHandle = xTaskCreateStatic(MSGEQ7_Task, "MSGEQ7",
			MSGEQ7_STACK_SIZE,
			NULL, configMAX_PRIORITIES - 1, msgeq7TaskStack,
			&msgeq7TaskTCBBuffer);

	for (i = 0; i < FREQUENCIES; i++) {
		pSample[i] = samples[i];
	}

	//NOTE timer period is not set and the timer is not started
	FTM_GetDefaultConfig(&ftm1_config);
	FTM_Init(FTM1, &ftm1_config);
	FTM_EnableInterrupts(FTM1, kFTM_TimeOverflowInterruptEnable);
	NVIC_SetPriority(FTM1_IRQn, 4);
	EnableIRQ(FTM1_IRQn);
}

/**
 * Main loop runs as fast as possible to keep the readings up to date for the display
 */
void MSGEQ7_Task(void *pvParameters) {
	uint32_t i;
	uint32_t j;
	uint32_t max;

	for (;;) {
		/**
		 * Toggle the reset pin to latch the msgeq7 analysis (rising-edge triggered)
		 * and wait 72us before hitting the strobe
		 */
		GPIOC->PSOR = (1 << MSGEQ7_Reset_PIN);
		GPIOC->PCOR = (1 << MSGEQ7_Reset_PIN);
		FTM1->MOD = COUNT_72US_AT_60MHz;
		FTM_StartTimer(FTM1, kFTM_SystemClock);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		for (i = 0; i < FREQUENCIES; i++) {
			/**
			 * Activate the strobe and wait 32us
			 * for the output to settle
			 */
			GPIOC->PCOR = (1 << MSGEQ7_Strobe_PIN);
			FTM1->MOD = COUNT_32US_AT_60MHz;
			FTM_StartTimer(FTM1, kFTM_SystemClock);
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

			/**
			 * Start the ADC conversion and wait for it to complete
			 */
			ADC16_SetChannelConfig(ADC16_1_PERIPHERAL, 0x00,
					&ADC16_1_channelsConfig[0]);
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

			if (pSample[i] == samples[i] + NUM_SAMPLES) {
				pSample[i] = samples[i];
			}

			*pSample[i]++ = adcConversionResult;
			max = samples[i][0];

			for (j = 1; j < NUM_SAMPLES; j++) {
				if (samples[i][j] > max) {
					max = samples[i][j];
				}
			}

			msgeq7Results[i] = max;
			/**
			 * Toggle the strobe and wait 72us as a minimum
			 * strobe-to-strobe delay
			 */
			GPIOC->PSOR = (1 << MSGEQ7_Strobe_PIN);
			if (i != FREQUENCY_16000Hz) {
				FTM1->MOD = COUNT_72US_AT_60MHz;
				FTM_StartTimer(FTM1, kFTM_SystemClock);
				ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			}
		}
	}
}

/**
 * clear the interrupt flag
 * clear the clock to stop the timer
 * reset the counter
 */
void FTM1_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	FTM1->SC &= ~(FTM_SC_TOF_MASK | FTM_SC_CLKS_MASK);
	FTM1->CNT = 0x00;
	vTaskNotifyGiveFromISR(msgeq7TaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * Reading the result clears the interrupt flag
 * It is done here because the OS does strange things when attempting to yield
 * while the interrupt appears to be pending.
 */
void ADC0_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	adcConversionResult = ADC0->R[0];

	vTaskNotifyGiveFromISR(msgeq7TaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
