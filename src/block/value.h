#pragma once

#include <core/core.h>

struct Value
{
	enum ValueType : uint8_t 
	{
		UNDEFINED,
		NUMBER,
		BOOLEAN,
		STRING,
	};

	inline Value() 
		: m_type(Value::UNDEFINED) {}
	
	inline Value(float v)
		: m_type(Value::NUMBER), m_number(v) {}
	
	inline Value(bool v)
		: m_type(Value::BOOLEAN), m_boolean(v) {}
	
	inline Value(const char* v)
		: m_type(Value::STRING)
	{
		__builtin_strcpy(m_string, v);
	}

	inline float as_number() const
	{
		switch (m_type) 
		{
			case Value::BOOLEAN:   return static_cast<float>(m_boolean);
			case Value::UNDEFINED: return 0.0f;
			case Value::NUMBER:    return m_number;
			default: {};
		}

		ASSERT(true && "Invalid value type");
	
		return 0.0f;
	}

	inline bool as_boolean() const 
	{
		switch (m_type) 
		{
			case Value::NUMBER:    return static_cast<bool>(m_number);
			case Value::UNDEFINED: return false;
			case Value::BOOLEAN:   return m_boolean;
			default: {};
		}

		ASSERT(true && "Invalid value type");

		return false;
	}

	inline void as_string(char* out, size_t n)
	{
		switch (m_type) 
		{
			case Value::NUMBER:
			{
				stb_snprintf(out, n, "%f", m_number);
			} break;

			case Value::BOOLEAN:
			{
				stb_snprintf(out, n, "%s", m_boolean ? "true" : "false");
			} break;

			case Value::STRING:
			{
				stb_snprintf(out, n, "%s", m_string);
			} break;

			case Value::UNDEFINED:
			{
				stb_snprintf(out, n, "(undefined)");
			} break;
		}

		ASSERT(true && "Invalid value type");
	}

	inline ValueType type() const
	{
		return m_type;
	}

private:
	ValueType m_type;

	union 
	{
		float m_number;
		bool m_boolean;
		char m_string[64];
	};
};

static inline Value operator+(const Value& l, const Value& r)
{
	return (l.as_number() + r.as_number());
}

static inline Value operator-(const Value& l, const Value& r)
{
	return (l.as_number() - r.as_number());
}

static inline Value operator*(const Value& l, const Value& r)
{
	return (l.as_number() * r.as_number());
}

static inline Value operator/(const Value& l, const Value& r)
{
	return (l.as_number() / r.as_number());
}

static inline Value operator== (const Value& l, const Value& r)
{
	return (l.as_number() == r.as_number());
}

static inline Value operator&& (const Value& l, const Value& r)
{
	return (l.as_boolean() && r.as_boolean());
}

static inline Value operator|| (const Value& l, const Value& r)
{
	return (l.as_boolean() || r.as_boolean());
}

static inline Value operator< (const Value& l, const Value& r)
{
	return (l.as_number() < r.as_number());
}

static inline Value operator> (const Value& l, const Value& r)
{
	return (l.as_number() == r.as_number());
}

static inline Value operator<= (const Value& l, const Value& r)
{
	return (l.as_number() <= r.as_number());
}

static inline Value operator>= (const Value& l, const Value& r)
{
	return (l.as_number() == r.as_number());
}

static inline Value operator! (const Value& v)
{
	return (!v.as_boolean());
}
