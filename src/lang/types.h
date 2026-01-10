#pragma once

#include <base/common.h>

//
// value
//

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

	inline Value& operator=(const Value& r) 
	{
		if (this == &r) return *this;

		type = r.type;

		switch (type) 
		{
			case VALUE_NUMBER:  number = r.number; break;
			case VALUE_BOOLEAN: boolean = r.boolean; break;
			case VALUE_STRING:  __builtin_strcpy(string, r.string); break;
				
			default: break;
		}

		return *this;
	}
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

		default: break;
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

		default: break;
	}

	ASSERT(true && "Invalid value type");

	return false;
}

inline void as_string(const Value v, char* out, size_t n)
{
	switch (v.type) 
	{
		case VALUE_NUMBER:    __builtin_snprintf(out, n, "%f", v.number); break;
		case VALUE_BOOLEAN:   __builtin_snprintf(out, n, "%s", v.boolean ? "true" : "false"); break;
		case VALUE_STRING:    __builtin_snprintf(out, n, "%s", v.string); break;
		case VALUE_UNDEFINED: __builtin_snprintf(out, n, "(undefined)"); break;
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

//
// instruction
//

#define DEFINE_INSTRUCTIONS \
	INST(CALL) \
	INST(LOAD_NULL) \
	INST(LOAD_IMMEDIATE) \
	INST(LOAD_LOCAL) \
	INST(STORE_LOCAL) \
	INST(JUMP) \
	INST(JUMP_COND) \
	INST(ADD) \
	INST(SUB) \
	INST(DIV) \
	INST(MUL) \
	INST(MOD) \
	INST(AND) \
	INST(OR) \
	INST(GREATER_THAN) \
	INST(LESS_THAN) \
	INST(EQUALS) \
	INST(GREATER) \
	INST(LESS) \
	INST(NOT) \
	INST(NEGATE) \
	INST(RETURN) \
	INST(HALT)

#undef INST
#define INST(name) INSTRUCTION_##name,

enum InstructionType : uint8_t
{
	DEFINE_INSTRUCTIONS
};

struct Instruction
{
	InstructionType type;
	uint16_t arg;
};

#undef INST
#define INST(name) case INSTRUCTION_##name: return #name;

inline const char* to_string(const InstructionType type)
{
	switch (type)
	{
		DEFINE_INSTRUCTIONS
	}
	
	return "unknown";
}

// the DEFINE_INSTRUCTIONS macro above is sorted in that way so this check is very easily done
inline bool needs_argument(const InstructionType type)
{
	return type >= INSTRUCTION_CALL && type <= INSTRUCTION_JUMP_COND;
}

//
// token
//

#define DEFINE_TOKENS \
	TOKEN(UNKNOWN,        "unknown") \
	\
	TOKEN(STRING,         "string literal") \
	TOKEN(NUMBER,         "number literal") \
	TOKEN(IDENTIFIER,     "identifier") \
	TOKEN(TRUE,           "`true`") \
	TOKEN(FALSE,          "`false`") \
	\
	TOKEN(VAR,            "keyword `var`") \
	TOKEN(FOREVER,        "keyword `forever`") \
	TOKEN(REPEAT,         "keyword `repeat`") \
	TOKEN(UNTIL,          "keyword `until`") \
	TOKEN(DEFINE,         "keyword `define`") \
	TOKEN(RETURN,         "keyword `return`") \
	TOKEN(IF,             "keyword `if`") \
	TOKEN(ELSE,           "keyword `else`") \
	TOKEN(BREAK,          "keyword `break`") \
	TOKEN(FOREIGN,        "keyword `foreign`") \
	\
	TOKEN(LESS,           "`<`") \
	TOKEN(GREATER,        "`>`") \
	TOKEN(LESS_THAN,      "`<=`") \
	TOKEN(GREATER_THAN,   "`>=`") \
	TOKEN(EQUALS,         "`=`") \
	TOKEN(DOUBLE_EQUALS,  "`==`") \
	TOKEN(EXCLAMATION,    "`!`") \
	TOKEN(NOT_EQUALS,     "`!=`") \
	TOKEN(DOUBLE_AND,     "`&&`") \
	TOKEN(DOUBLE_PIPE,    "`||`") \
	TOKEN(PERCENT,        "`%`") \
	TOKEN(PERCENT_EQUALS, "`%=`") \
	TOKEN(PLUS,           "`+`") \
	TOKEN(PLUS_EQUALS,    "`+=`") \
	TOKEN(DASH,           "`-`") \
	TOKEN(DASH_EQUALS,    "`-=`") \
	TOKEN(SLASH,          "`/`") \
	TOKEN(SLASH_EQUALS,   "`/=`") \
	TOKEN(STAR,           "`*`") \
	TOKEN(STAR_EQUALS,    "`*=`") \
	\
	TOKEN(OPEN_BRACE,     "`{`") \
	TOKEN(CLOSE_BRACE,    "`}`") \
	TOKEN(OPEN_PAREN,     "`(`") \
	TOKEN(CLOSE_PAREN,    "`)`") \
	TOKEN(COMMA,          "`,`")

#undef TOKEN
#define TOKEN(name, fancy_name) TOKEN_##name,

enum TokenType : uint8_t 
{	
	DEFINE_TOKENS
};

struct Token
{
	TokenType type;
	uint16_t line; 
	uint32_t start;
	uint32_t end;  
};

#undef TOKEN
#define TOKEN(name, fancy_name) case TOKEN_##name: return fancy_name;

inline const char* to_string(const TokenType type)
{
	switch (type)
	{
		DEFINE_TOKENS
	}
	
	return "unknown";
}

inline void as_string(Token t, const char* src, char* out, size_t n)
{
	__builtin_snprintf(out, __builtin_elementwise_min(t.end - t.start + 1, static_cast<uint32_t>(n)), "%s", src + t.start);
}

inline bool is_binary_op(const TokenType type)
{
	return type >= TOKEN_LESS && type <= TOKEN_STAR_EQUALS;
}

inline bool is_assign_op(const TokenType type)
{
	return type == TOKEN_PLUS_EQUALS || 
		type == TOKEN_DASH_EQUALS || 
		type == TOKEN_SLASH_EQUALS || 
		type == TOKEN_STAR_EQUALS || 
		type == TOKEN_PERCENT_EQUALS || 
		type == TOKEN_EQUALS;	
}

// the precedences mostly follow C's.
// see: https://en.cppreference.com/w/c/language/operator_precedence.html
// goes from lowest from highest level of precedence
inline int8_t precedence_of(const TokenType type)
{
	switch (type)
	{
		case TOKEN_DOUBLE_AND: return 0;
		case TOKEN_DOUBLE_PIPE: return 1;

		case TOKEN_NOT_EQUALS:
		case TOKEN_DOUBLE_EQUALS: return 2;  

		case TOKEN_LESS:
		case TOKEN_GREATER:
		case TOKEN_LESS_THAN:
		case TOKEN_GREATER_THAN: return 3;

		case TOKEN_PLUS:
		case TOKEN_DASH: return 4;

		case TOKEN_PERCENT:
		case TOKEN_SLASH:
		case TOKEN_STAR: return 5;

		default: {}
	}

	return -1;
}