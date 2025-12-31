#pragma once

#include <base/common.h>

#include <base/util.h>

#include <base/array.h>

struct Token
{
	enum TokenType : uint8_t 
	{
		UNKNOWN,

		STRING_LIT,
		NUMBER_LIT,
		IDENTIFIER,
		VAR,
		FOREVER,

		EQUALS,
		OPEN_CURLY,
		CLOSE_CURLY,
		OPEN_BRACE,
		CLOSE_BRACE,
	};

	inline Token() = default;

	inline Token(TokenType type, uint64_t start = 0, uint64_t end = 0)
		: type(type), start(start), end(end) {}

	inline const char* as_string() const
	{
		switch (type) 
		{
			case Token::UNKNOWN:     return "unknown";
			case Token::STRING_LIT:  return "string literal";
			case Token::NUMBER_LIT:  return "number literal";
			case Token::IDENTIFIER:  return "identifier";
			case Token::VAR:         return "keyword `var`";
			case Token::FOREVER:     return "keyword `forever`";
			case Token::EQUALS:      return "`=`";
			case Token::OPEN_CURLY:  return "`{`";
			case Token::CLOSE_CURLY: return "`}`";
			case Token::OPEN_BRACE:  return "`(`";
			case Token::CLOSE_BRACE: return "`)`";
		}
	
		return "(unknown)";
	}

	inline void value(const char* src, char* out, size_t n) const
	{
		__builtin_snprintf(out, __builtin_elementwise_min(end - start + 1, static_cast<uint64_t>(n)), "%s", src + start);
	}

	TokenType type;

	uint64_t start;
	uint64_t end;
};

struct Tokenizer
{
public:
	Tokenizer(const char* src);

	template <size_t N>
	void tokenize(Array <Token, N>& tokens);

private:
	Token parse_identifier();

	Token parse_string();

	Token parse_number();

	char peek(uint64_t offset = 0);

	char eat();

private:
	const char* m_src;
	uint64_t m_line;
	uint64_t m_idx;
};