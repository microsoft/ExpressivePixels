// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

enum VariantCapabilities
{
	VARIANTCAPABILITY_STORAGE = 0x1,
	VARIANTCAPABILITY_PREVIEW = 0x2,
};


#define EPX_DEVICEMODEL			"CircuitEPXDev"
#define BLE_DEFAULT_DEVICE_NAME "CircuitEPX"                               /**< Name of device. Will be included in the advertising data. */
#define EPX_FWVERSIONSRC 		""

#define GPIO_PIN_STATUSLED		13
#define GPIO_PIN_FLASHRAM_CS	0
#define GPIO_PIN_FLASHRAM_MISO	0
#define GPIO_PIN_FLASHRAM_SCLK	0
#define GPIO_PIN_FLASHRAM_MOSI	0
#define GPIO_PIN_FEATURE		0
#define GPIO_PIN_3V3_ACCEN		0
#define GPIO_PIN_BOOSTER_ENABLE 0
#define DISPLAYARRAY_DATAPIN	9
#define DISPLAYARRAY_POWERPIN	21
#define FLASH_SPI_CS			SS					 // Flash chip SS pin.
#define FLASH_SPI_PORT			SPI                   // What SPI port is Flash on?
#define VARIANT_CAPABILITIES	(VARIANTCAPABILITY_STORAGE | VARIANTCAPABILITY_PREVIEW)

extern char g_szDEFAULT_BLE_NAME[];
