// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXApp.h"
EPX_OPTIMIZEFORDEBUGGING_ON

// #define DOTSTAR8X8_DISPLAY
#ifdef DOTSTAR8X8_DISPLAY
#include "AdafruitDotStar_EPXDriver.h"

#define DISPLAYARRAY_WIDTH		8
#define DISPLAYARRAY_HEIGHT		8

uint16_t g_displayArrayPixelTopology[DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT] = 
{ 
	7,  6,  5,  4,  3,  2,  1,  0, 
    15, 14, 13, 12, 11, 10, 9,  8,
    23, 22, 21, 20, 19, 18, 17, 16,
    31, 30, 29, 28, 27, 26, 25, 24,
    39, 38, 37, 36, 35, 34, 33, 32,
    47, 46, 45, 44, 43, 42, 41, 40,
    55, 54, 53, 52, 51, 50, 49, 48, 
    63, 62, 61, 60, 59, 58, 57, 56
};

CAdafruitDotStar_EPXDriver dotStarDriver(PIN_A3, PIN_A2, DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT);
void *g_pLEDDriver = &dotStarDriver;

#else
#include "AdafruitNeoPixel_EPXDriver.h"

#define DISPLAYARRAY_WIDTH		16
#define DISPLAYARRAY_HEIGHT		16

uint16_t g_displayArrayPixelTopology[DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT] = 
{ 
	0,  31, 32, 63, 64, 95, 96, 127,128,159,160,191,192,223,224,255,
	1,  30, 33, 62, 65, 94, 97, 126,129,158,161,190,193,222,225,254,
	2,  29, 34, 61, 66, 93, 98, 125,130,157,162,189,194,221,226,253,
	3,  28, 35, 60, 67, 92, 99, 124,131,156,163,188,195,220,227,252,
	4,  27, 36, 59, 68, 91,100, 123,132,155,164,187,196,219,228,251,
	5,  26, 37, 58, 69, 90,101, 122,133,154,165,186,197,218,229,250,
	6,  25, 38, 57, 70, 89,102, 121,134,153,166,185,198,217,230,249,
	7,  24, 39, 56, 71, 88,103, 120,135,152,167,184,199,216,231,248,
	8,  23, 40, 55, 72, 87,104, 119,136,151,168,183,200,215,232,247,
	9,  22, 41, 54, 73, 86,105, 118,137,150,169,182,201,214,233,246,
	10, 21, 42, 53, 74, 85,106, 117,138,149,170,181,202,213,234,245,
	11, 20, 43, 52, 75, 84,107, 116,139,148,171,180,203,212,235,244,
	12, 19, 44, 51, 76, 83,108, 115,140,147,172,179,204,211,236,243,
	13, 18, 45, 50, 77, 82,109, 114,141,146,173,178,205,210,237,242,
	14, 17, 46, 51, 78, 81,110, 113,142,145,174,177,206,209,238,241,
	15, 16, 47, 48, 79, 80,111, 112,143,144,175,176,207,208,239,240
};

CAdafruitNeoPixel_EPXDriver neoPixel_EPXDriver(PIN_A3, DISPLAYARRAY_WIDTH * DISPLAYARRAY_HEIGHT);
void *g_pLEDDriver = &neoPixel_EPXDriver;
#endif

char g_szDEFAULT_BLE_NAME[BLEMAX_DEVICENAME + 1] = "AdafruitCPX";
EPX_GUID g_displayDesignGUID = { 0x39, 0xF2, 0xB0, 0x5D, 0x86, 0x8D, 0x49, 0x42, 0x99, 0x44, 0x31, 0xE0, 0x85, 0xF0, 0x13, 0x15 };



void setup() 
{
    EPXPlatform_Runtime_Initialize();
    EPXApp_Initialize(g_pLEDDriver, g_displayArrayPixelTopology, DISPLAYARRAY_WIDTH, DISPLAYARRAY_HEIGHT);
}



void loop() 
{
	// Process main app
    EPXPlatform_Runtime_Process();
	EPXApp_Process();
}

