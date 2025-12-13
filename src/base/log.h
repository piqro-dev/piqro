#pragma once

#include <base/common.h>

static inline void println(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	int len = __builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	out[len] = '\n';
	out[len + 1] = '\0';

	wchar_t w_out[2048];

	MultiByteToWideChar(CP_UTF8, 0, out, -1, w_out, 2048);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), w_out, __builtin_wcslen(w_out) + 1, nullptr, nullptr);
}

static inline void errorln(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_strcpy(out, "ERROR: ");

	__builtin_va_start(args, fmt);
	int len = __builtin_vsprintf(out + __builtin_strlen(out), fmt, args);
	__builtin_va_end(args);

	out[len] = '\n';
	out[len + 1] = '\0';

	wchar_t w_out[2048];

	MultiByteToWideChar(CP_UTF8, 0, out, -1, w_out, 2048);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), w_out, __builtin_wcslen(w_out) + 1, nullptr, nullptr);
}