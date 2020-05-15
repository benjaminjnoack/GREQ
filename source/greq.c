#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK66F18.h"
#include "fsl_debug_console.h"

#include "static_allocation.h"
#include "run_time_stats.h"

#include <ht16k33.h>
#include <max7219.h>
#include <msgeq7.h>
#include <potentiometer.h>
#include <rotary_encoder.h>

int main(void) {
	/* Prevent GPIO IRQs from firing before the RTOS can handle them */
	DisableIRQ(PORTB_IRQn);
	DisableIRQ(PORTD_IRQn);
	/* Initialize board hardware */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Initialize FSL debug console */
	BOARD_InitDebugConsole();

	PRINTF("GREQ\n");

	POTENTIOMETER_Init();
	ROTARY_ENCODER_Init();
	MSGEQ7_Init();
	HT16K33_Init();
	MAX7219_Init();

	vTaskStartScheduler();

	//should never get here...
	while (1) {
		__NOP();
	}
	//really, this should never happen...
	return 0;
}
