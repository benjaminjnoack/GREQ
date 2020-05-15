/*
 * rotary_encoder.c
 *
 *  Created on: Jan 3, 2019
 *      Author: ben
 */

#include <rotary_encoder.h>

#define ROTARY_ENCODER_STACK_SIZE configMINIMAL_STACK_SIZE

static StaticTask_t rotaryEncoderTaskTCBBuffer;
static StackType_t rotaryEncoderTaskStack[ROTARY_ENCODER_STACK_SIZE];

EventGroupHandle_t rotaryEncoderEventGroupHandle;
volatile int currentVolume = 0x00;

void ROTARY_ENCODER_Task(void *pvParameters);

void ROTARY_ENCODER_Init(void) {

	rotaryEncoderEventGroupHandle = xEventGroupCreate();
	configASSERT(rotaryEncoderEventGroupHandle);

	xTaskCreateStatic(ROTARY_ENCODER_Task, "ROTARY_ENCODER", ROTARY_ENCODER_STACK_SIZE, NULL,
			configMAX_PRIORITIES - 1, rotaryEncoderTaskStack, &rotaryEncoderTaskTCBBuffer);

	NVIC_SetPriority(PORTB_IRQn, 4);
	NVIC_SetPriority(PORTD_IRQn, 4);
	EnableIRQ(PORTB_IRQn);
	EnableIRQ(PORTD_IRQn);
}

void ROTARY_ENCODER_Task(void *pvParameters) {
	static EventBits_t bits;

	for (;;) {
		bits = xEventGroupWaitBits(rotaryEncoderEventGroupHandle,
				(ROTARY_ENCODER_LEFT | ROTARY_ENCODER_RIGHT | ROTARY_ENCODER_CLICK), pdTRUE, pdFALSE,
				portMAX_DELAY);

		if (bits & ROTARY_ENCODER_LEFT) {
			GPIOE->PSOR = (1 << BOARD_LED_GREEN_GPIO_PIN);
			GPIOA->PTOR = (1 << BOARD_LED_BLUE_GPIO_PIN);
		} else if (bits & ROTARY_ENCODER_RIGHT) {
			GPIOA->PSOR = (1 << BOARD_LED_BLUE_GPIO_PIN);
			GPIOE->PTOR = (1 << BOARD_LED_GREEN_GPIO_PIN);
		} else if (bits & ROTARY_ENCODER_CLICK) {
			if (GPIOD->PDIR & (1 << ROTARY_ENCODER_BUTTON_PIN)) {
				GPIOC->PCOR = (1 << BOARD_LED_RED_GPIO_PIN);
			} else {
				GPIOC->PSOR = (1 << BOARD_LED_RED_GPIO_PIN);
			}
		}
	}
}

/**
 * The two phases of the rotary encoder are attached to PORTB
 */
void PORTB_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static BaseType_t xResult;
	static uint32_t interrupts = 0;

	interrupts = PORTB->ISFR;

	//the interrupt was caused by phase A rising
	if (interrupts & (1 << ROTARY_ENCODER_PHASE_A_PIN)) {
		PORTB->ISFR = (1 << ROTARY_ENCODER_PHASE_A_PIN);

		if (currentVolume == MIN_VOLUME) {
			return;
		}

		//phase B is also high
		if (GPIOB->PDIR & (1 << ROTARY_ENCODER_PHASE_B_PIN)) {
			--currentVolume;
			xResult = xEventGroupSetBitsFromISR(rotaryEncoderEventGroupHandle,
			ROTARY_ENCODER_LEFT, &xHigherPriorityTaskWoken);

			if (xResult != pdFAIL) {
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	} else if (interrupts & (1 << ROTARY_ENCODER_PHASE_B_PIN)) {
		PORTB->ISFR = (1 << ROTARY_ENCODER_PHASE_B_PIN);

		if (currentVolume == MAX_VOLUME) {
			return;
		}

		if (GPIOB->PDIR & (1 << ROTARY_ENCODER_PHASE_A_PIN)) {
			++currentVolume;
			xResult = xEventGroupSetBitsFromISR(rotaryEncoderEventGroupHandle,
			ROTARY_ENCODER_RIGHT, &xHigherPriorityTaskWoken);

			if (xResult != pdFAIL) {
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
}

/**
 * The button on the rotary encoder is attached to PORTD
 */
void PORTD_IRQHandler(void) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (PORTD->ISFR == (1U << ROTARY_ENCODER_BUTTON_PIN)) {
		PORTD->ISFR = (1U << ROTARY_ENCODER_BUTTON_PIN);

		xEventGroupSetBitsFromISR(rotaryEncoderEventGroupHandle, ROTARY_ENCODER_CLICK,
				&xHigherPriorityTaskWoken);
	}
}
