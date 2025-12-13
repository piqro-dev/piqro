#pragma once

#include <base/common.h>

namespace os 
{

static inline void exit(int32_t exit_code);

enum DialogFlags : uint32_t
{
	ICON_ERROR = 1 << 0,
	ICON_INFO = 1 << 1,
};

static inline void dialog(const char* message, const char* caption, uint32_t flags);

}