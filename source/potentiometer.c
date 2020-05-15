/*
 * potentiometer.c
 *
 *  Created on: Dec 29, 2018
 *      Author: ben
 */

#include <potentiometer.h>

#define TIMER_BIT 0x01
#define ADC_BIT 0x02
#define POTENTIOMETER_STACK_SIZE configMINIMAL_STACK_SIZE

static StaticTask_t potentiometerTaskTCBBuffer;
static StackType_t potentiometerStack[POTENTIOMETER_STACK_SIZE];
static TaskHandle_t potentiometerTaskHandle;
static EventGroupHandle_t potentiometerEventGroup;

volatile uint32_t adc1ConversionResult;
uint32_t potentiometerResults[5];

void POTENTIOMETER_Task(void *pvParameters);

/**
 * create the task
 */
void POTENTIOMETER_Init(void) {
	ftm_config_t ftm2_config;

	potentiometerEventGroup = xEventGroupCreate();
	configASSERT(potentiometerEventGroup);

	potentiometerTaskHandle = xTaskCreateStatic(POTENTIOMETER_Task, "POTENTIOMETER",
			POTENTIOMETER_STACK_SIZE, NULL, configMAX_PRIORITIES - 1,
			potentiometerStack, &potentiometerTaskTCBBuffer);

	FTM_GetDefaultConfig(&ftm2_config);
	ftm2_config.prescale = kFTM_Prescale_Divide_32;
	FTM_Init(FTM2, &ftm2_config);
	FTM_EnableInterrupts(FTM2, kFTM_TimeOverflowInterruptEnable);
	FTM_SetTimerPeriod(FTM2, 30000);
	NVIC_SetPriority(FTM2_IRQn, 4);
	EnableIRQ(FTM2_IRQn);
	FTM_StartTimer(FTM2, kFTM_SystemClock);
}

/**
 * wait for notification that the conversion is done
 * start the next conversion in the array
 */
void POTENTIOMETER_Task(void *pvParameters) {
	static uint32_t i;

	for (;;) {

		for (i = 0; i < 5; i++) {
			xEventGroupWaitBits(potentiometerEventGroup, TIMER_BIT, pdTRUE, pdTRUE,
					portMAX_DELAY);
			ADC16_SetChannelConfig(ADC1, 0x00, &ADC16_2_channelsConfig[i]);

			xEventGroupWaitBits(potentiometerEventGroup, ADC_BIT, pdTRUE, pdTRUE,
					portMAX_DELAY);
			potentiometerResults[i] = adc1ConversionResult;
		}
	}
}

/**
 * Read the result into a volatile variable to clear the flag
 * notify the task to continue with business
 */
void ADC1_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static BaseType_t xResult;

	adc1ConversionResult = ADC1->R[0];

	xResult = xEventGroupSetBitsFromISR(potentiometerEventGroup, ADC_BIT,
			&xHigherPriorityTaskWoken);

	if (xResult != pdFAIL) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void FTM2_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken;
	static BaseType_t xResult;

	FTM2->SC &= ~FTM_SC_TOF_MASK;

	xResult = xEventGroupSetBitsFromISR(potentiometerEventGroup, TIMER_BIT,
			&xHigherPriorityTaskWoken);

	if (xResult != pdFAIL) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
