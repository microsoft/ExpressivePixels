// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef USESEGGERRTT_LOG
#include <SEGGER_RTT.h>
#endif

EPX_OPTIMIZEFORDEBUGGING_ON

#define DFU_MAGIC_OTA_RESET             0xA8
#define DFU_MAGIC_SERIAL_ONLY_RESET     0x4e
#define DFU_MAGIC_UF2_RESET             0x57


void EPXPlatform_Runtime_Reboot(uint8_t rebootType)
{
	switch (rebootType)
	{
	case REBOOTTYPE_UF2:
		// sd_power_gpregret_set(0, DFU_MAGIC_UF2_RESET);
		DEBUGLOGLN("System_Reboot DFU_MAGIC_UF2_RESET");
		break;
		
	case REBOOTTYPE_OTA:
		// sd_power_gpregret_set(0, DFU_MAGIC_OTA_RESET);
		DEBUGLOGLN("System_Reboot DFU_MAGIC_OTA_RESET");
		break;
		
	case REBOOTTYPE_SERIAL:
		// sd_power_gpregret_set(0, DFU_MAGIC_SERIAL_ONLY_RESET);
		DEBUGLOGLN("System_Reboot DFU_MAGIC_SERIAL_ONLY_RESET");
		break;
		
	default:
		DEBUGLOGLN("System_Reboot NORMAL");
		break;
	}	
	delay(500);
	NVIC_SystemReset();
}



extern "C" 
{	
	/*********************************************************************************/
	/* Centralize heap functions so they can be swapped for tracing/tracking purposes
	/*********************************************************************************/
	void *tmalloc(const char *pszFile, int line, size_t siz)
	{
	return malloc(siz);
	}



	void tfree(void *buf)
	{
	free(buf);
	}



	void tmallocstats()
	{
	}



	void tmallocdump()
	{
	}



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



    char *epx_strupr(char *s)
	{
		char *pCur, *pNew;

		pNew = (char*)TMALLOC(strlen(s));
		if (pNew != NULL)
		{
			pCur = pNew;
			while (*s != 0x00)
			{
				*pCur = toupper(*s);
				s++;
				pCur++;
			}
			*pCur = 0x00;
		}
		return pNew;
	}	/* END STRUPR */



	uint32_t millisPassed(uint32_t localMillis) 
	{
		uint32_t currentMillis = millis();
		return currentMillis - localMillis;
	}
	

	
	void EPXPlatform_Runtime_Initialize()
	{
		#ifdef USESEGGERRTT_LOG
		SEGGER_RTT_Init();
		SEGGER_RTT_WriteString(0, ">> SEGGER_RTT Initialized **\r\n");
		#endif
	}


	void EPXPlatform_Runtime_Process()
	{
		
	}



	#ifdef USESEGGERRTT_LOG

	void SEGGER_RTT_LogLn(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		int size = vsnprintf(NULL, 0, fmt, args) + 1;
		char* buf = (char*)malloc(size);
		vsnprintf(buf, size, fmt, args);
		SEGGER_RTT_WriteString(0, buf);
		SEGGER_RTT_WriteString(0, "\r\n");
		free(buf);
		va_end(args);
	}
	#endif
}



void EPXPlatform_Runtime_ControlAppTimer(bool on)
{

}



void EPXPlatform_Runtime_MCUSleep()
{
//	DEBUGLOGLN("Sleeping...");
		    		   	
//	DEBUGLOGLN("Awake!!");
}



void EPXPlatform_Runtime_MCUDeepSleep()
{
	
}



#ifdef ARDUINO_ARCH_SAMD
extern "C" char *sbrk(int i);

int freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
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


int freeRam() {
	char stack_dummy = 0;
	return (int)((uint64_t)((void*) &stack_dummy) - (uint64_t) _sbrk(0));
}

#endif


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

