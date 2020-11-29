// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_GPIO.h"
#include "EPXPlatform_Runtime.h"

EPX_OPTIMIZEFORDEBUGGING_ON

#define MAX_GPIO_EVENTHANDLERS	4
#define MAX_APP_BUTTONS			4

void _btnCallback1();
void _btnCallback2();
void _btnCallback3();
void _btnCallback4();

typedef void (*PFNINTERRUPTCALLBACK)(void);

typedef struct
{
	PFNINTERRUPTCALLBACK callback;
	uint32_t ulPin;
} _APPBTNCONFIG;

_APPBTNCONFIG g_appBtnConfig[MAX_APP_BUTTONS] = { { _btnCallback1, 0}, {_btnCallback2, 0}, {_btnCallback3, 0}, {_btnCallback4, 0} };



static uint8_t			 		g_numConfiguredAppButtons = 0;
static void						*g_gpioHostInstances[MAX_GPIO_EVENTHANDLERS] = { NULL, NULL, NULL, NULL };
static PFN_EPXGPIO_EVENTHANDLER g_gpioEventHandlers[MAX_GPIO_EVENTHANDLERS] = { NULL, NULL, NULL, NULL };


	
void EPXPlatform_GPIO_PinConfigure(uint32_t ulPin, uint32_t type)
{
	pinMode(ulPin, type);
}



uint32_t EPXPlatform_GPIO_PinRead(uint32_t ulPin)
{
	return digitalRead(ulPin);
}



void EPXPlatform_GPIO_PinWrite(uint32_t ulPin, uint32_t ulVal)
{
	digitalWrite(ulPin, ulVal);
}



bool EPXPlatform_GPIO_Initialize(void *pinstance, PFN_EPXGPIO_EVENTHANDLER eventHandler)
{
for (int idx = 0; idx < MAX_GPIO_EVENTHANDLERS; idx++)
	{
		if (g_gpioEventHandlers[idx] == NULL)
		{
			g_gpioHostInstances[idx] = pinstance;
			g_gpioEventHandlers[idx] = eventHandler;
			return true;
		}
	}
	return false;
}



void EPXPlatform_GPIO_ButtonClickConfigure(uint32_t ulPin)
{
	if (g_numConfiguredAppButtons < MAX_APP_BUTTONS)
	{		
		pinMode(ulPin, INPUT_PULLUP);
		g_appBtnConfig[g_numConfiguredAppButtons].ulPin = ulPin;
		attachInterrupt(digitalPinToInterrupt(ulPin), g_appBtnConfig[g_numConfiguredAppButtons].callback, FALLING);
		g_numConfiguredAppButtons++;
	}
}



void EPXPlatform_GPIO_ButtonClickFinalize()
{
}



void _btnCallback1()
{
	DEBUGLOGLN("GPIO_PIN %d pushed", g_appBtnConfig[0].ulPin);
	for (int idx = 0; idx < MAX_GPIO_EVENTHANDLERS; idx++)
	{
		if (g_gpioEventHandlers[idx] != NULL)
			(*g_gpioEventHandlers[idx])(g_gpioHostInstances[idx], EPXGPIO_BUTTON_PUSHED, g_appBtnConfig[0].ulPin, 1);
	}
}



void _btnCallback2()
{
	DEBUGLOGLN("GPIO_PIN %d pushed", g_appBtnConfig[1].ulPin);
	for (int idx = 0; idx < MAX_GPIO_EVENTHANDLERS; idx++)
	{
		if (g_gpioEventHandlers[idx] != NULL)
			(*g_gpioEventHandlers[idx])(g_gpioHostInstances[idx], EPXGPIO_BUTTON_PUSHED, g_appBtnConfig[1].ulPin, 1);
	}
}



void _btnCallback3()
{
	DEBUGLOGLN("GPIO_PIN %d pushed", g_appBtnConfig[2].ulPin);
	for (int idx = 0; idx < MAX_GPIO_EVENTHANDLERS; idx++)
	{
		if (g_gpioEventHandlers[idx] != NULL)
			(*g_gpioEventHandlers[idx])(g_gpioHostInstances[idx], EPXGPIO_BUTTON_PUSHED, g_appBtnConfig[2].ulPin, 1);
	}

}



void _btnCallback4()
{
	DEBUGLOGLN("GPIO_PIN %d pushed", g_appBtnConfig[3].ulPin);
	for (int idx = 0; idx < MAX_GPIO_EVENTHANDLERS; idx++)
	{
		if (g_gpioEventHandlers[idx] != NULL)
			(*g_gpioEventHandlers[idx])(g_gpioHostInstances[idx], EPXGPIO_BUTTON_PUSHED, g_appBtnConfig[3].ulPin, 1);
	}
}

