// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Platform implementation for I2C capability on Nordic SDK
 *
 **/
#pragma once
#include "app_util_platform.h"



bool		EPXPlatform_I2S_Initialize();
bool		EPXPlatform_I2S_Start(void *p, size_t cb);
void		EPXPlatform_I2S_Configure(uint16_t pin);
void		EPXPlatform_I2S_UnInitialize();
void		EPXPlatform_I2S_Stop();

