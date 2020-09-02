// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_GPIO.h"
#include "EPXPlatform_Runtime.h"

static void						*g_gpioHostInstance = NULL;
static PFN_EPXGPIO_EVENTHANDLER g_gpioEventHandler = NULL;
	
	
void EPXPlatform_GPIO_PinConfigure(uint32_t ulPin, uint32_t type)
{
	pinMode(ulPin, type);
}



void EPXPlatform_GPIO_PinWrite(uint32_t ulPin, uint32_t ulVal)
{
	digitalWrite(ulPin, ulVal);
}



void EPXPlatform_GPIO_Initialize(void *pinstance, PFN_EPXGPIO_EVENTHANDLER eventHandler)
{
	g_gpioHostInstance = pinstance;
	g_gpioEventHandler = eventHandler;
}



void EPXPlatform_GPIO_ButtonClickConfigure(uint32_t ulPin)
{
}


