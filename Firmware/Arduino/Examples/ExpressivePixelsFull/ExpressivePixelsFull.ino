// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXApp.h"
#include "EPXApp_Trigger_Switch.h"
EPX_OPTIMIZEFORDEBUGGING_ON

/************************** Configuration ***********************************/

// edit the config.h tab and configure your display
#include "config.h"

char g_szDEFAULT_BLE_NAME[BLEMAX_DEVICENAME + 1] = "EPXArduino";
EPX_GUID g_displayDesignGUID = { 0x39, 0xF2, 0xB0, 0x5D, 0xAA, 0x8D, 0x49, 0x42, 0x99, 0x44, 0x31, 0xE0, 0x85, 0xF0, 0x13, 0x15 };



void setup() 
{
	void *g_pLEDDriver = NULL;

    // Create the underlying driver class
    #ifdef DISPLAYDRIVER_ADAFRUIT_DOTSTAR
      // For a APA102 type of LED
      CAdafruitDotStar_EPXDriver *pDotStarDriver = new CAdafruitDotStar_EPXDriver(PIN_STRIP_DATA, PIN_STRIP_CLOCK, DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT);
      g_pLEDDriver = pDotStarDriver;
    #elif defined(DISPLAYDRIVER_ADAFRUIT_NEOPIXEL)
      // For WS2812 LEDs
      CAdafruitNeoPixel_EPXDriver *pNeoPixelDriver = new CAdafruitNeoPixel_EPXDriver(PIN_STRIP_DATA, DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT);
      g_pLEDDriver = pNeoPixelDriver;
    #elif defined(DISPLAYDRIVER_ADAFRUIT_PROTOMATTER)
      // For Protomatter RGB Matrixes
      CAdafruitRGBMatrix_EPXDriver *pRGBMatrixDriver = new CAdafruitRGBMatrix_EPXDriver(DISPLAYARRAY_WIDTH, DISPLAYARRAY_HEIGHT);
      g_pLEDDriver = pRGBMatrixDriver;
    #endif

#if defined(DISPLAY_SPARKLETSQUARE16X16) || defined(DISPLAY_MATRIX64x32)
	for(int i = 0;i < DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT;i++)
		g_displayArrayPixelTopology[i]   = i;
#endif

    EPXPlatform_Runtime_Initialize();
    CSwitchActivation::SubsystemInitialize(NUM_SWITCHTRIGGER_PINS, g_switchActivationMapping);
    EPXApp_Initialize(g_pLEDDriver, g_displayArrayPixelTopology, DISPLAYARRAY_WIDTH, DISPLAYARRAY_HEIGHT);
}



void loop() 
{
    // Process main app
    EPXPlatform_Runtime_Process();
    EPXApp_Process();
}
