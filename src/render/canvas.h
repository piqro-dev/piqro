#pragma once

#include <base/common.h>

static constexpr uint8_t MAX_WIDTH = 240;
static constexpr uint8_t MAX_HEIGHT = 320;

struct Canvas
{
	uint32_t buffer[MAX_WIDTH * MAX_HEIGHT];
};