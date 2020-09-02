// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "app_util_platform.h"
#include "nrf_drv_timer.h"
#include "nrf_delay.h"
#include "nrf_timer.h"
#include "nrf_drv_wdt.h"
#include <string.h>
#include <ctype.h>
#include "EPXPlatform_Runtime.h"
#include "EPXPlatform_GPIO.h"
extern "C"
{
	#include "nrf_pwr_mgmt.h"
}


uint32_t			g_AppTimerTick = 0;
nrf_drv_timer_t		g_appTimer;


#define DFU_MAGIC_OTA_RESET             0xA8
#define DFU_MAGIC_SERIAL_ONLY_RESET     0x4e
#define DFU_MAGIC_UF2_RESET             0x57

void _EPXPlatform_Runtime_InitializeAppTimer();



void EPXPlatform_Runtime_Reboot(uint8_t rebootType)
{
	switch (rebootType)
	{
	case REBOOTTYPE_UF2:
		sd_power_gpregret_set(0, DFU_MAGIC_UF2_RESET);
		DEBUGLOGLN("System_Reboot DFU_MAGIC_UF2_RESET");
		break;
		
	case REBOOTTYPE_OTA:
		sd_power_gpregret_set(0, DFU_MAGIC_OTA_RESET);
		DEBUGLOGLN("System_Reboot DFU_MAGIC_OTA_RESET");
		break;
		
	case REBOOTTYPE_SERIAL:
		sd_power_gpregret_set(0, DFU_MAGIC_SERIAL_ONLY_RESET);
		DEBUGLOGLN("System_Reboot DFU_MAGIC_SERIAL_ONLY_RESET");
		break;
		
	default:
		DEBUGLOGLN("System_Reboot NORMAL");
		break;
	}	
	delay(500);
	NVIC_SystemReset();
}



static void _WatchdogEventHandler()
{
	DEBUGLOGLN("*** WATCHDOG TRIGGERED ***");
	NVIC_SystemReset();
}


static bool g_bWatchdogInitialized = false;
static nrfx_wdt_channel_id g_watchDogChannelID;
void _EPXPlatform_Runtime_InitializeWatchdog()
{
	nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
	uint32_t err_code = nrf_drv_wdt_init(&config, _WatchdogEventHandler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_wdt_channel_alloc(&g_watchDogChannelID);
	APP_ERROR_CHECK(err_code);
	nrf_drv_wdt_enable();  
	g_bWatchdogInitialized = true;
}



void _EPXPlatform_Runtime_WatchdogFeed()
{	
	if (g_bWatchdogInitialized)
		nrf_drv_wdt_channel_feed(g_watchDogChannelID);
}



extern "C" 
{
	
	char *stristr(const char *subject, const char *object) 
	{
		int c = tolower(*object);
		if (c == 0x00)
			return (char *)subject;
		for (; *subject; subject++)
		{
			if (tolower(*subject) == c)
			{
				for (size_t i = 0;;) 
				{
					if (object[++i] == 0x00)
						return (char *)subject;
					if (tolower(subject[i]) != tolower(object[i]))
						break;
				}
			}
		}
		return NULL;
	}



	int stricmp(const char *s1, const char *s2)
	{
		while (1) 
		{
			char c1 = *s1++;
			char c2 = *s2++;
			int diff = tolower(c1) - tolower(c2);
			if (diff == 0) 
			{
				if (c1 == 0)
					return 0;			
			}
			else
				return diff;
		}
		return 0;
	}

	
	
	char *epx_strupr(char s[])
	{
		char	*p;

		for (p = s; *p; ++p)
			*p = toupper(*p);
		return (s);
	}	/* END STRUPR */
	
	
	
	uint32_t millis()
	{
		return g_AppTimerTick;
	}



        unsigned long micros()
        {
            return millis();
        }



	#define OVERFLOWMILLIS ((uint32_t)(0xFFFFFFFF/32.768))

	uint32_t millisPassed(uint32_t localMillis) 
	{
		uint32_t currentMillis = millis();
		if (currentMillis < localMillis)
			return currentMillis + OVERFLOWMILLIS + 1 - localMillis;
		else
			return currentMillis - localMillis;
	}
	


	void delay(uint32_t ms)
	{
		nrf_delay_ms(ms);   	
	}
	
	
	
	void delayMicroseconds(uint32_t us)
	{
		nrf_delay_us(us);
	}
	

	
	void EPXPlatform_Runtime_Initialize()
	{
		_EPXPlatform_Runtime_InitializeAppTimer();
#ifdef USEWATCHDOG
		_EPXPlatform_Runtime_InitializeWatchdog();
#endif
	}

	
	
	void EPXPlatform_Runtime_Process()
	{
		_EPXPlatform_Runtime_WatchdogFeed();
	}
	
}



void _EPXPlatform_Runtime_AppTimerHandler(nrf_timer_event_t event_type, void* p_context) 
{
	g_AppTimerTick++;
}




void _EPXPlatform_Runtime_InitializeAppTimer()
{
	g_appTimer.p_reg = NRF_TIMER1;
	g_appTimer.instance_id = NRFX_TIMER1_INST_IDX;
	g_appTimer.cc_channel_count = NRF_TIMER_CC_CHANNEL_COUNT(1);
	
	nrf_drv_timer_config_t timerConfig = NRF_DRV_TIMER_DEFAULT_CONFIG;
	timerConfig.p_context = NULL;
	timerConfig.frequency = NRF_TIMER_FREQ_31250Hz;
	uint32_t err_code = nrf_drv_timer_init(&g_appTimer, &timerConfig, _EPXPlatform_Runtime_AppTimerHandler);
	APP_ERROR_CHECK(err_code);
    
	nrf_drv_timer_extended_compare(&g_appTimer, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_ms_to_ticks(&g_appTimer, 1), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);    
	nrf_drv_timer_enable(&g_appTimer);
}



void EPXPlatform_Runtime_ControlAppTimer(bool on)
{
	if (on)
		nrf_drv_timer_enable(&g_appTimer);
	else
		nrf_drv_timer_disable(&g_appTimer);
}



void EPXPlatform_Runtime_MCUSleep()
{
	NRF_LOG_INFO("Sleeping...");
	EPXPlatform_Runtime_ControlAppTimer(false);
		    		   
	/*
	uint32_t err_code = sd_app_evt_wait();
	APP_ERROR_CHECK(err_code);*/

	
	nrf_pwr_mgmt_run();
	
	NRF_LOG_INFO("Awake!!");
	EPXPlatform_Runtime_ControlAppTimer(true);		  
}




void EPXPlatform_Runtime_MCUDeepSleep()
{
	NRF_LOG_INFO("Deep Sleeping...");
	EPXPlatform_Runtime_ControlAppTimer(false);
		    		   
	int ret = sd_power_system_off();
		
	NRF_LOG_INFO("Deep Sleep Awake!! %d", ret);
	EPXPlatform_Runtime_ControlAppTimer(true);		  
}



#ifdef FREERAM
#ifdef ARDUINO_ARCH_SAMD
extern "C" char *sbrk(int i);

int freeRam() {
	char stack_dummy = 0;
	return &stack_dummy - sbrk(0);
}
#else


void* _sbrk(ptrdiff_t incr) {
	extern uint32_t __HeapBase;
	extern uint32_t __HeapLimit;
	static char* heap = 0;
	if (heap == 0) heap = (char*)&__HeapBase;
	void* ret = heap;
	if (heap + incr >= (char*)&__HeapLimit)
		ret = (void*) - 1;
	else
		heap += incr;
	return ret;
}
#endif

int freeRam() {
	char stack_dummy = 0;
	return (int)((uint64_t)((void*) &stack_dummy) - (uint64_t) _sbrk(0));
}
#endif


int freeRam() {
  return 0;
}



uint8_t HexToByte(char *hex, int len)
{
	int i = len > 1 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X') ? 2 : 0;
	uint8_t value = 0;

	while (i < len)
	{
		uint8_t x = hex[i++];

		if (x >= '0' && x <= '9') x = x - '0';
		else if (x >= 'A' && x <= 'F') x = (x - 'A') + 10;
		else if (x >= 'a' && x <= 'f') x = (x - 'a') + 10;
		else return 0;

		value = 16 * value + x;
	}

	return value;
}



void BytesToHex(uint8_t *p, int cb, char *psz, int stringCB)
{
	int stringOffset = 0;
	
	while (cb > 0 && stringCB >= 2)	
	{
		sprintf(psz + stringOffset, "%02X", *p);
		p++;
		stringOffset += 2;
		cb--;
		stringCB--;
	}		
	psz[stringOffset] = 0x00;
}

