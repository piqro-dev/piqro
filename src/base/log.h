#pragma once

#include <base/common.h>

#if defined __wasm__
	extern "C" void js_console_log(const char *);
	extern "C" void js_console_error(const char *);
#endif

__attribute__((format(printf, 1, 2)))
inline void println(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	#if defined __wasm__
		js_console_log(out);
	#elif defined _WIN32
		__builtin_printf("%s\n", out);
	#endif
}

__attribute__((format(printf, 1, 2)))
inline void errorln(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	#if defined __wasm__
		js_console_error(out);
		__builtin_trap();
	#elif defined _WIN32
		__builtin_printf("\e[91m%s\e[0m\n", out);
		exit(1);
	#endif
}