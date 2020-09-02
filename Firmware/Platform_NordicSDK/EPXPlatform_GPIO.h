// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/*
 * Platform implementation for GPIO capability on Nordic SDK
 *
 **/

#pragma once
#include "app_util_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

enum EPXGPIOStateChange
{
	EPXGPIO_PIN_STATECHANGE = 0,
	EPXGPIO_BUTTON_PUSHED,
	EPXGPIO_BUTTON_RELEASED
};
	
#define EPXGPIO_LOW             (0x0)
#define EPXGPIO_HIGH            (0x1)
#define EPXGPIO_INPUT           (0x0)
#define EPXGPIO_OUTPUT          (0x1)

	
typedef void(*PFN_EPXGPIO_EVENTHANDLER)(void *pinstance, uint8_t event, uint16_t pin, uint16_t value);	
	
typedef void(*PFN_EPX_BUTTON_PUSHED)(void *pinstance);	

void EPXPlatform_GPIO_ButtonClickConfigure(uint32_t ulPin);
void EPXPlatform_GPIO_ButtonClickFinalize();
bool EPXPlatform_GPIO_Initialize(void *pinstance, PFN_EPXGPIO_EVENTHANDLER eventHandler);
uint32_t EPXPlatform_GPIO_PinRead(uint32_t ulPin);
void EPXPlatform_GPIO_PinConfigure(uint32_t ulPin, uint32_t type);	
void EPXPlatform_GPIO_PinWrite(uint32_t ulPin, uint32_t ulVal);

	
// Compatability with Arduino libraries	
#define LOW						EPXGPIO_LOW
#define HIGH					EPXGPIO_HIGH
#define INPUT					EPXGPIO_INPUT
#define OUTPUT					EPXGPIO_OUTPUT
	
#define digitalWrite(ulPin, ulVal) EPXPlatform_GPIO_PinWrite(ulPin, ulVal)
#define pinMode(ulPin, ulVal) EPXPlatform_GPIO_PinConfigure(ulPin, ulVal)		
	
#ifdef __cplusplus
}
#endif

