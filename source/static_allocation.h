/*
 * static_allocation.h
 *
 *  Created on: Jan 19, 2019
 *      Author: ben
 */

#ifndef STATIC_ALLOCATION_H_
#define STATIC_ALLOCATION_H_

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize);

#endif /* STATIC_ALLOCATION_H_ */
