/*
 * static_allocation.c
 *
 *  Created on: Jan 19, 2019
 *      Author: ben
 *
 *  https://www.freertos.org/a00110.html#configSUPPORT_STATIC_ALLOCATION
 */

#include "static_allocation.h"

#define IDLE_TASK_SIZE configMINIMAL_STACK_SIZE
#define TIMER_TASK_SIZE configMINIMAL_STACK_SIZE

static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[IDLE_TASK_SIZE];
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[TIMER_TASK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;
	*pulIdleTaskStackSize = IDLE_TASK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pulTimerTaskStackSize = TIMER_TASK_SIZE;

}
