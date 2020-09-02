// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include "EPXPlatform_Runtime.h"

bool		EPXPlatform_I2S_Initialize();
bool		EPXPlatform_I2S_Start(void *p, size_t cb);
void		EPXPlatform_I2S_Configure(uint16_t pin);
void		EPXPlatform_I2S_UnInitialize();
void		EPXPlatform_I2S_Stop();

