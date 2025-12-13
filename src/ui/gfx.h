#pragma once

#include <base/common.h>

#include <ui/window.h>

namespace gfx
{

enum TextureId : uint16_t
{
	INVALID = static_cast<uint16_t>(-1)
};

static inline TextureId create_texture(const uint8_t* data, uint16_t w, uint16_t h);

static inline void init();

static inline bool is_ready();

static inline void clear(float r, float g, float b, float a);

static inline void begin();

static inline void present();

static inline void resize_viewport();

static inline void fill_rect();

}