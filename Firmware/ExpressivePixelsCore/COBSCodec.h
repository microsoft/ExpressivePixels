// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include <stdint.h>
#include <stddef.h>

extern "C"
{
	size_t COBS_Encode(const uint8_t * input, size_t length, uint8_t * output);
	size_t COBS_Decode(const uint8_t * input, size_t length, uint8_t * output);

}


