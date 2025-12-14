#pragma once

// Taken from https://qiita.com/benikabocha/items/e943deb299d0f816f161#utf-8-%E3%81%8B%E3%82%89-utf-32

#include <base/common.h>

static inline size_t get_utf8_byte_count(char8_t c) 
{
	if (0 <= c && c < 0x80)
	{
		return 1;
	}
	
	if (0xC2 <= c && c < 0xE0) 
	{
		return 2;
	}
	
	if (0xE0 <= c && c < 0xF0) 
	{
		return 3;
	}
	
	if (0xF0 <= c && c < 0xF8) 
	{
		return 4;
	}

	return 0;
}

static inline bool is_utf8_later(char8_t c)
{
	return 0x80 <= c && c < 0xC0;
}

static inline bool utf_8_to_32(char utf8[4], char32_t* utf32)
{
	const size_t num = get_utf8_byte_count(utf8[0]);
	
	if (num == 0) 
	{
		return false;
	}
	
	switch (num) 
	{
		case 1:
		{
			*utf32 = (char32_t)((char8_t)utf8[0]);
		} break;

		case 2:
		{
			if (!is_utf8_later(utf8[1])) 
			{
				return false;
			}

			if ((char8_t)((utf8[0]) & 0x1E) == 0) 
			{
				return false;
			}

			*utf32 = (char32_t)(utf8[0] & 0x1F) << 6;
			*utf32 |= (char32_t)(utf8[1] & 0x3F);
		} break;

		case 3:
		{
			if (!is_utf8_later(utf8[1]) || !is_utf8_later(utf8[2])) 
			{
				return false;
			}

			if ((char8_t)(utf8[0] & 0x0F) == 0 && (char8_t)(utf8[1] & 0x20) == 0) 
			{
				return false;
			}

			*utf32 = (char32_t)(utf8[0] & 0x0F) << 12;
			*utf32 |= (char32_t)(utf8[1] & 0x3F) << 6;
			*utf32 |= (char32_t)(utf8[2] & 0x3F);
		} break;
		
		case 4:
		{
			if (!is_utf8_later(utf8[1]) || !is_utf8_later(utf8[2]) || !is_utf8_later(utf8[3])) 
			{
				return false;
			}

			if (((char8_t)utf8[0] & 0x07) == 0 && ((char8_t)utf8[1] & 0x30) == 0) 
			{
				return false;
			}

			*utf32 = (char32_t)(utf8[0] & 0x07) << 18;
			*utf32 |= (char32_t)(utf8[1] & 0x3F) << 12;
			*utf32 |= (char32_t)(utf8[2] & 0x3F) << 6;
			*utf32 |= (char32_t)(utf8[3] & 0x3F);
		} break;
			
		default: return false;
	}

	return true;
}