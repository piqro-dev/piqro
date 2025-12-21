#pragma once

#include <base/common.h>

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_DECORATE(name) name

#include <third_party/stb_sprintf.h>

#if defined __wasm__
	namespace js { extern "C" void console_log(const char *); }
	namespace js { extern "C" void console_error(const char *); }
#endif

__attribute__((format(printf, 1, 2)))
static inline void println(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	#if defined __wasm__
		js::console_log(out);
	#endif
}

__attribute__((format(printf, 1, 2)))
static inline void errorln(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	#if defined __wasm__
		js::console_error(out);
	#endif
}