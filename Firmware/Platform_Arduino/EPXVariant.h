// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#define EPX_DEVICEMODEL			"EPXArduino"
#define BLE_DEFAULT_DEVICE_NAME "EPXArduino"                               /**< Name of device. Will be included in the advertising data. */
#define EPX_FWVERSIONSRC 		""

#define GPIO_PIN_STATUSLED		13
#define GPIO_PIN_FLASHRAM_CS	0
#define GPIO_PIN_FLASHRAM_MISO	0
#define GPIO_PIN_FLASHRAM_SCLK	0
#define GPIO_PIN_FLASHRAM_MOSI	0
#define GPIO_PIN_FEATURE		0
#define GPIO_PIN_3V3_ACCEN		0
#define GPIO_PIN_BOOSTER_ENABLE 0
#define DISPLAYARRAY_POWERPIN	0
#define FLASH_SPI_CS			SS					 // Flash chip SS pin.
#define FLASH_SPI_PORT			SPI                   // What SPI port is Flash on?

#define VARIANTCAPABILITY_STORAGE
#define VARIANTCAPABILITY_PREVIEW
#define VARIANTCAPABILITY_BATTERY_MONITORING
// #define VARIANTCAPABILITY_SECURITY
// #define VARIANTCAPABILITY_DFU
// #define VARIANT_DISPLAY_PWRMGNT	

extern char g_szDEFAULT_BLE_NAME[];
