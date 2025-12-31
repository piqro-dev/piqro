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
	#elif defined _WIN32
		__builtin_strcat(out, "\n");

		wchar_t w_out[2048];
		MultiByteToWideChar(CP_UTF8, 0, out, -1, w_out, 2048);
		
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), w_out, __builtin_wcslen(w_out), nullptr, nullptr);
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
		__builtin_trap();
	#elif defined _WIN32
		__builtin_strcat(out, "\n");
		
		wchar_t w_out[2048];
		MultiByteToWideChar(CP_UTF8, 0, out, -1, w_out, 2048);

		const HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(std_out, &csbi);

    SetConsoleTextAttribute(std_out, FOREGROUND_RED | FOREGROUND_INTENSITY);
		WriteConsoleW(std_out, w_out, __builtin_wcslen(w_out), nullptr, nullptr);
		SetConsoleTextAttribute(std_out, csbi.wAttributes);
	
		ExitProcess(1);
	#endif
}