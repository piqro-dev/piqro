#pragma once

#include <base/common.h>

__attribute__((format(printf, 1, 2)))
static inline void println(const char* fmt, ...)
{
	va_list args;

	char out[2048];

	va_start(args, fmt);
	vsprintf(out, fmt, args);
	va_end(args);

	#if defined __wasm__
		extern void js_console_log(const char*);
		js_console_log(out);
	#elif defined _WIN32
		printf("%s\n", out);
	#endif
}

__attribute__((format(printf, 1, 2)))
static inline void errorln(const char* fmt, ...)
{
	va_list args;

	char out[2048];

	va_start(args, fmt);
	vsprintf(out, fmt, args);
	va_end(args);

	#if defined __wasm__
		extern void js_console_error(const char*);
		js_console_error(out);
	#elif defined _WIN32
		printf("%s\n", out);
	#endif
}