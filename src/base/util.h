#pragma once

static inline bool is_alpha(int ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static inline bool is_number(int ch)
{
	return (ch >= '0' && ch <= '9');
}

static inline bool is_alnum(int ch)
{
	return is_alpha(ch) || is_number(ch);
}