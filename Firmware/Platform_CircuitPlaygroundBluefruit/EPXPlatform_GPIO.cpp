// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_GPIO.h"
#include "EPXPlatform_Runtime.h"

EPX_OPTIMIZEFORDEBUGGING_ON

#define MAX_GPIO_EVENTHANDLERS	4
#define MAX_APP_BUTTONS			4

static uint8_t			 		g_numConfiguredAppButtons = 0;
//static app_button_cfg_t 		g_appButtons[MAX_APP_BUTTONS];
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
		// g_appButtons[g_numConfiguredAppButtons].pin_no = (uint8_t) ulPin;
		// g_appButtons[g_numConfiguredAppButtons].active_state = APP_BUTTON_ACTIVE_LOW;
		//g_appButtons[g_numConfiguredAppButtons].pull_cfg = NRF_GPIO_PIN_PULLUP;
		// g_appButtons[g_numConfiguredAppButtons].button_handler = appbutton_event_handler;
		g_numConfiguredAppButtons++;
	}
}



void EPXPlatform_GPIO_ButtonClickFinalize()
{
}

