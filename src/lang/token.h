#pragma once

#define DEFINE_TOKENS \
	TOKEN(UNKNOWN,     "unknown") \
	\
	TOKEN(STRING_LIT,  "string literal") \
	TOKEN(NUMBER_LIT,  "number literal") \
	TOKEN(IDENTIFIER,  "identifier") \
	\
	TOKEN(VAR,         "keyword `var`") \
	TOKEN(FOREVER,     "keyword `forever`") \
	TOKEN(REPEAT,      "keyword `repeat`") \
	\
	TOKEN(EQUALS,      "`=`") \
	TOKEN(OPEN_CURLY,  "`{`") \
	TOKEN(CLOSE_CURLY, "`}`") \
	TOKEN(OPEN_BRACE,  "`(`") \
	TOKEN(CLOSE_BRACE, "`)`")

#define TOKEN(name, fancy_name) name,

struct Token
{
	enum TokenType : uint8_t 
	{	
		DEFINE_TOKENS
	};

	inline Token() = default;

	inline Token(TokenType type, uint64_t start = 0, uint64_t end = 0)
		: type(type), start(start), end(end) {}

	#undef TOKEN
	#define TOKEN(name, fancy_name) case name: return fancy_name;

	inline const char* as_string() const
	{
		switch (type) 
		{
			DEFINE_TOKENS
		}
	
		return "unknown";
	}

	inline void value(const char* src, char* out, size_t n) const
	{
		__builtin_snprintf(out, __builtin_elementwise_min(end - start + 1, static_cast<uint64_t>(n)), "%s", src + start);
	}

	TokenType type;

	uint64_t start;
	uint64_t end;
};