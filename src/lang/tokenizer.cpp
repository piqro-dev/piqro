#include <lang/tokenizer.h>

Tokenizer::Tokenizer(const char* src)
	: m_src(src), m_idx(0) {}

template <size_t N>
void Tokenizer::tokenize(Array <Token, N>& tokens)
{
	const size_t src_len = __builtin_strlen(m_src);

	while (m_idx < src_len)
	{
		switch (peek()) 
		{
			case '\n':
			{
				m_line++;	
			}
			case '\r':
			case '\t':
			case ' ':
			{
				eat();
			} break;

			case '{':
			{
				tokens.emplace(Token::OPEN_CURLY, m_idx, m_idx + 1);
				eat();
			} break;

			case '}':
			{
				tokens.emplace(Token::CLOSE_CURLY, m_idx, m_idx + 1);
				eat();
			} break;

			case '(':
			{
				tokens.emplace(Token::OPEN_BRACE, m_idx, m_idx + 1);
				eat();
			} break;

			case ')':
			{
				tokens.emplace(Token::CLOSE_BRACE, m_idx, m_idx + 1);		
				eat();
			} break;

			case '=':
			{
				tokens.emplace(Token::EQUALS, m_idx, m_idx + 1);	
				eat();
			} break;

			case '\'':
			{
				tokens.emplace(parse_string());
			} break;

			default:
			{
				if (is_alpha(peek()))
				{
					tokens.push(parse_identifier());
				}
				// Allow the first character to be the decimal point (so numbers like .321 are allowed)
				else if (is_number(peek()) || peek() == '.') 
				{
					tokens.push(parse_number());
				}
			} break;
		}
	}
}

static constexpr struct 
{
	Token::TokenType type;
	const char*	name;
} KEYWORDS[] =
{
	{ Token::VAR,     "var" },
	{ Token::FOREVER, "forever" },
	{ Token::REPEAT,  "repeat" },
};

Token Tokenizer::parse_identifier()
{
	Token t = { Token::IDENTIFIER };
	
	t.start = m_idx;

	eat();
	
	while (is_alnum(peek())) 
	{ 
		eat(); 
	}
	
	t.end = m_idx;

	// Reassign type if it is a keyword
	const auto equals = [&](const char* name)
	{
		char buf[128] = {};
		__builtin_snprintf(buf, t.end - t.start + 1, "%s", m_src + t.start);

		return __builtin_strcmp(buf, name) == 0;
	};

	for (const auto& name : KEYWORDS)
	{
		if (equals(name.name))
		{
			t.type = name.type;
			break;
		}
	}

	return t;
}

Token Tokenizer::parse_string()
{
	Token t = { Token::STRING_LIT };

	t.start = m_idx;

	eat();

	while (eat() != '\'')
	{
		if (!peek())
		{
			errorln("Error: On line %llu: Expected `'` to close string literal", m_line);
		}
	}

	t.end = m_idx;

	return t;
}

Token Tokenizer::parse_number()
{
	Token t = { Token::NUMBER_LIT };

	t.start = m_idx;

	bool decimal = false;

	// First character may be the decimal point
	if (peek() == '.')
	{
		decimal = true;

		// Bail out early if the next character isn't a number
		if (!is_number(peek(1))) 
		{
			// TODO: Better diagnostics, something like "unexpected `.` near <token>""
			errorln("Error: On line %llu: Unexpected `.`", m_line);
		}
	}

	eat();

	while (is_number(peek()) || peek() == '.')
	{ 
		if (peek() == '.')
		{
			if (!decimal)
			{
				decimal = true;
			}
			else
			{
				errorln("Error: On line %llu: Unexpected `.` in number literal", m_line);
			}
		} 

		eat();
	}
	
	t.end = m_idx;

	return t;
}

char Tokenizer::peek(uint64_t offset)
{
	if (m_idx + offset > __builtin_strlen(m_src)) 
	{
		return 0;
	}

	return m_src[m_idx + offset];
}

char Tokenizer::eat()
{
	return m_src[m_idx++];
}
