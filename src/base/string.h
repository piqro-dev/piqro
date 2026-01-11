#pragma once

#include <base/common.h>

#include <base/arena.h>

typedef struct String String;
struct String
{
	char* buffer;
	size_t length;
};

#define s(s) ((String){ s, sizeof(s) - 1 })

#define s_fmt(s) (int)s.length, s.buffer

static inline String str_copy_c_str(Arena* arena, const char* c_str)
{
	String s = {};

	const char* p = c_str;

	while (*p++)
	{
		s.length++;
	}

	s.buffer = arena_push_array(arena, char, s.length);

	for (size_t i = 0; i < s.length; i++)
	{
		s.buffer[i] = c_str[i];
	}

	return s;
}

static inline String str_copy(Arena* arena, String str)
{
	String s = {};

	s.length = str.length;
	s.buffer = arena_push_array(arena, char, s.length);

	for (size_t i = 0; i < s.length; i++)
	{
		s.buffer[i] = str.buffer[i];
	}

	return s;
}

static inline String str_copy_from_to(Arena* arena, String str, size_t a, size_t b)
{
	String s = {};

	s.length = b - a;
	s.buffer = arena_push_array(arena, char, s.length);

	for (size_t i = a; i < b; i++)
	{
		s.buffer[i - a] = str.buffer[i];
	}

	return s;
}

static inline String str_copy_c_str_from_to(Arena* arena, const char* str, size_t a, size_t b)
{
	String s = {};

	s.length = b - a;
	s.buffer = arena_push_array(arena, char, s.length);

	for (size_t i = a; i < b; i++)
	{
		s.buffer[i - a] = str[i];
	}

	return s;
}

static inline float str_as_number(String s)
{
	char out[2048];

	for (size_t i = 0; i < s.length; i++)
	{
		out[i] = s.buffer[i];
	}

	out[s.length] = '\0';

	return atof(out);
}

static inline String str_format(Arena* arena, const char* fmt, ...)
{
	char out[2048];

	va_list args;

	va_start(args, fmt);
	vsnprintf(out, sizeof(out), fmt, args);
	va_end(args);

	return str_copy_c_str(arena, out);
}

static inline bool str_equals(String l, String r)
{
	if (l.length != r.length)
	{
		return false;
	}

	for (size_t i = 0; i < l.length; i++)
	{
		if (l.buffer[i] != r.buffer[i])
		{
			return false;
		}
	}

	return true;
}

//
// util
//

static inline bool is_alpha(int32_t ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static inline bool is_number(int32_t ch)
{
	return (ch >= '0' && ch <= '9');
}

static inline bool is_alnum(int32_t ch)
{
	return is_alpha(ch) || is_number(ch);
}