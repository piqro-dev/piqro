#pragma once

#include <base/common.h>

namespace window
{

static void* handle;

static uvec2 size;

static struct 
{
	ivec2 client;
	ivec2 movement;
} mouse;

static bool open;

static void (*on_resize)();

static void (*on_paint)();

static inline void init(const char* title);

static inline void poll_events();

}