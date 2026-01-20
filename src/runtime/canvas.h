#pragma once

#include <base/common.h>
#include <base/arena.h>

#include <runtime/config.h>

typedef struct RT_Canvas RT_Canvas;
struct RT_Canvas
{
	uint8_t* buffer;
	uint16_t width;
	uint16_t height;
};

#define R_COLOR(r, g, b) (((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6))

RT_Canvas canvas_make(Arena* arena, uint16_t width, uint16_t height);

void rt_canvas_clear(RT_Canvas* c, uint8_t color);

void rt_canvas_rect(RT_Canvas* c, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

void rt_canvas_put(RT_Canvas* c, int16_t x, int16_t y, uint8_t color);