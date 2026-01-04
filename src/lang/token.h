#pragma once

#include <base/common.h>

#define DEFINE_TOKENS \
	TOKEN(UNKNOWN,     "unknown") \
	\
	TOKEN(STRING,      "string literal") \
	TOKEN(NUMBER,      "number literal") \
	TOKEN(IDENTIFIER,  "identifier") \
	TOKEN(TRUE,        "`true`") \
	TOKEN(FALSE,       "`false`") \
	\
	TOKEN(VAR,         "keyword `var`") \
	TOKEN(FOREVER,     "keyword `forever`") \
	TOKEN(REPEAT,      "keyword `repeat`") \
	TOKEN(DEFINE,      "keyword `define`") \
	TOKEN(RETURN,      "keyword `return`") \
	TOKEN(IF,          "keyword `if`") \
	TOKEN(ELSE,        "keyword `else`") \
	\
	TOKEN(PLUS,        "`+`") \
	TOKEN(DASH,        "`-`") \
	TOKEN(SLASH,       "`/`") \
	TOKEN(STAR,        "`*`") \
	TOKEN(EQUALS,      "`=`") \
	TOKEN(OPEN_CURLY,  "`{`") \
	TOKEN(CLOSE_CURLY, "`}`") \
	TOKEN(OPEN_BRACE,  "`(`") \
	TOKEN(CLOSE_BRACE, "`)`") \
	TOKEN(COMMA,       "`,`")

#undef TOKEN
#define TOKEN(name, fancy_name) TOKEN_##name,

enum TokenType : uint8_t 
{	
	DEFINE_TOKENS
};

struct Token
{
	TokenType type; // Type of the token
	uint16_t line;  // What line is it on, starts at 1
	uint32_t start; // The starting index in the source file 
	uint32_t end;   // The ending index of the token
};

#undef TOKEN
#define TOKEN(name, fancy_name) case TOKEN_##name: return fancy_name;

static inline const char* to_string(const TokenType type)
{
	switch (type)
	{
		DEFINE_TOKENS
	}
	
	return "unknown";
}

static inline void as_string(Token t, const char* src, char* out, size_t n)
{
	__builtin_snprintf(out, __builtin_elementwise_min(t.end - t.start + 1, static_cast<uint32_t>(n)), "%s", src + t.start);
}

static inline bool is_binary_op(const TokenType type)
{
	return type >= TOKEN_PLUS && type <= TOKEN_STAR;
}

static inline uint8_t precedence_of(const TokenType type)
{
	switch (type)
	{
		case TOKEN_PLUS:
		case TOKEN_DASH: return 0;

		case TOKEN_SLASH:
		case TOKEN_STAR: return 1;

		default: {};
		// TODO: ( ... )
	}

	return (uint8_t)-1;
}