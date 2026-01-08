#pragma once

#include <base/common.h>

enum ValueType : uint8_t 
{
	VALUE_UNDEFINED,
	VALUE_NUMBER,
	VALUE_BOOLEAN,
	VALUE_STRING,
};

static constexpr uint16_t MAX_VALUE_STRING_LENGTH = 64;

struct Value
{
	ValueType type;

	union
	{
		float number;
		bool boolean;
		char string[MAX_VALUE_STRING_LENGTH];
	};
};

inline Value make_value()
{
	Value value = {};

	value.type = VALUE_UNDEFINED;

	return value;
}

inline Value make_value(float v)
{
	Value value = {};

	value.type = VALUE_NUMBER;
	value.number = v;

	return value;
}

inline Value make_value(bool v)
{
	Value value = {};

	value.type = VALUE_BOOLEAN;
	value.boolean = v;

	return value;
}

inline Value make_value(const char* v)
{
	Value value = {};

	value.type = VALUE_STRING;
	__builtin_strcpy(value.string, v);

	return value;
}

inline float as_number(const Value v)
{
	switch (v.type) 
	{
		case VALUE_BOOLEAN:   return (float)v.boolean;
		case VALUE_UNDEFINED: return 0.0f;
		case VALUE_NUMBER:    return v.number;
		default: {};
	}

	ASSERT(true && "Invalid value type");

	return 0.0f;
}

inline bool as_boolean(const Value v)
{
	switch (v.type) 
	{
		case VALUE_NUMBER:    return (bool)v.number;
		case VALUE_UNDEFINED: return false;
		case VALUE_BOOLEAN:   return v.boolean;
		default: {};
	}

	ASSERT(true && "Invalid value type");

	return false;
}

inline void as_string(const Value v, char* out, size_t n)
{
	switch (v.type) 
	{
		case VALUE_NUMBER:
		{
			__builtin_snprintf(out, n, "%f", v.number);
		} break;

		case VALUE_BOOLEAN:
		{
			__builtin_snprintf(out, n, "%s", v.boolean ? "true" : "false");
		} break;

		case VALUE_STRING:
		{
			__builtin_snprintf(out, n, "%s", v.string);
		} break;

		case VALUE_UNDEFINED:
		{
			__builtin_snprintf(out, n, "(undefined)");
		} break;
	}

	ASSERT(true && "Invalid value type");
}

inline Value operator+(const Value& l, const Value& r)
{
	return make_value(as_number(l) + as_number(r));
}

inline Value operator-(const Value& l, const Value& r)
{
	return make_value(as_number(l) - as_number(r));
}

inline Value operator/(const Value& l, const Value& r)
{
	return make_value(as_number(l) / as_number(r));
}

inline Value operator*(const Value& l, const Value& r)
{
	return make_value(as_number(l) * as_number(r));
}

inline Value operator%(const Value& l, const Value& r)
{
	return make_value(__builtin_fmodf(as_number(l), as_number(r)));
}

inline Value operator==(const Value& l, const Value& r)
{
	return make_value(as_number(l) == as_number(r));
}

inline Value operator&&(const Value& l, const Value& r)
{
	return make_value(as_boolean(l) && as_boolean(r));
}

inline Value operator||(const Value& l, const Value& r)
{
	return make_value(as_boolean(l) || as_boolean(r));
}

inline Value operator<(const Value& l, const Value& r)
{
	return make_value(as_number(l) < as_number(r));
}

inline Value operator>(const Value& l, const Value& r)
{
	return make_value(as_number(l) > as_number(r));
}

inline Value operator<=(const Value& l, const Value& r)
{
	return make_value(as_number(l) <= as_number(r));
}

inline Value operator>=(const Value& l, const Value& r)
{
	return make_value(as_number(l) >= as_number(r));
}

inline Value operator!(const Value& v)
{
	return make_value(!as_boolean(v));
}
