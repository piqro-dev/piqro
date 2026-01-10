#pragma once

inline bool is_alpha(int ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

inline bool is_number(int ch)
{
	return (ch >= '0' && ch <= '9');
}

inline bool is_alnum(int ch)
{
	return is_alpha(ch) || is_number(ch);
}