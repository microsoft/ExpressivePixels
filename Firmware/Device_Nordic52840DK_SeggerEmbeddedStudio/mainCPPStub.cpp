#include "EPXApp.h"
#ifdef VARIANTDISPLAYDRIVER_I2SWS2812
#include "CWS2812-I2SDriver.h"
#elif defined(VARIANTDISPLAYDRIVER_ADANEOPIXEL)
#include "Adafruit_NeopixelDriver.h"
#endif

extern SWITCHACTIVATION_GPIOBUTTON_MAPPING g_switchTriggerGPIOButtonMappings[];
extern uint16_t g_displayArrayPixelTopology[];

extern "C"
{
	void NRF52_EPXApp_Initialize()
	{
                void *pDisplayDriver = NULL;

#if defined(DISPLAYTYPE_16x16) || defined(DISPLAYTYPE_22x22)
                for(int i = 0;i < DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT;i++)
                    g_displayArrayPixelTopology[i] = i;

#endif


#ifdef VARIANTDISPLAYDRIVER_I2SWS2812
		CWS2812_I2SDriver *pCWS2812_I2S_Driver = new CWS2812_I2SDriver(DISPLAYARRAY_DATAPIN, DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT);			
                pDisplayDriver = pCWS2812_I2S_Driver;
#elif defined(VARIANTDISPLAYDRIVER_ADANEOPIXEL)
                Adafruit_NeopixelDriver *pAdafruitNeopixelDriver = new Adafruit_NeopixelDriver(DISPLAYARRAY_DATAPIN, DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT);	
                pDisplayDriver = pAdafruitNeopixelDriver;
#endif

		CSwitchActivation::SubsystemInitialize(VARIANT_NUMTRIGGERGPIOS, g_switchTriggerGPIOButtonMappings);		
		EPXApp_Initialize(pDisplayDriver, g_displayArrayPixelTopology, DISPLAYARRAY_WIDTH, DISPLAYARRAY_HEIGHT);
	}
	

	
	void NRF52_EPXApp_Process()
	{
		EPXApp_Process();
	}
}

