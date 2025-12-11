#pragma once

#include <stdint.h>

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_DECORATE(name) stb_##name
#include <stb_sprintf.h>

using vec2 = float __attribute__((ext_vector_type(2)));
using vec4 = float __attribute__((ext_vector_type(4)));

#if defined __wasm__
	namespace js { extern "C" void console_log(const char*); }
	namespace js { extern "C" void console_error(const char*); }
#endif

static inline void println(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	stb_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	#if defined __wasm__
		js::console_log(out);
	#else
		__builtin_printf("%s\n", out);
	#endif
}

static inline void errorln(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	stb_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	#if defined __wasm__
		js::console_error(out);
	#else
		__builtin_printf("%s\n", out);
	#endif
}

#define ASSERT(expr) do { if (!(expr)) { errorln("Assertion failed: %s, in file: %s, line: %d", #expr, __FILE__, __LINE__); __builtin_unreachable(); } } while (0) 

#define DO_ONCE for (static bool once = false; !once; once = true)

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))