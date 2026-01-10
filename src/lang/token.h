#pragma once

#include <base/common.h>

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