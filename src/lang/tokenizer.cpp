#include <lang/tokenizer.h>

#include <lang/tokenizer.h>

inline char peek(const Tokenizer* tok, int16_t offset = 0) 
{
	if (tok->ptr + offset > __builtin_strlen(tok->source)) 
	{
		return 0;
	}

	return tok->source[tok->ptr + offset];
}

inline char eat(Tokenizer* tok)
{
	return tok->source[tok->ptr++];
}

__attribute__((format(printf, 2, 3)))
inline void error(const Tokenizer* tok, const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	errorln("Error: line %d: %s", tok->line, out);
}

static constexpr struct 
{
	TokenType type;
	const char*	name;
} KEYWORDS[] =
{
	{ TOKEN_TRUE,    "true" },
	{ TOKEN_FALSE,   "false" },

	{ TOKEN_VAR,     "var" },
	{ TOKEN_FOREVER, "forever" },
	{ TOKEN_REPEAT,  "repeat" },
	{ TOKEN_UNTIL,   "until" },
	{ TOKEN_DEFINE,  "define" },
	{ TOKEN_RETURN,  "return" },
	{ TOKEN_IF,      "if" },
	{ TOKEN_ELSE,    "else" },
	{ TOKEN_BREAK,   "break" },
};

inline Token parse_identifier(Tokenizer* tok)
{
	Token t = {};
	
	t.type = TOKEN_IDENTIFIER;
	t.start = tok->ptr;
	t.line = tok->line;

	eat(tok);
	
	while (is_alnum(peek(tok)) || peek(tok) == '_') 
	{ 
		eat(tok); 
	}
	
	t.end = tok->ptr;

	// reassign type if it is a keyword
	for (const auto& name : KEYWORDS)
	{
		char buf[128] = {};
		as_string(t, tok->source, buf, 128);

		if (__builtin_strcmp(buf, name.name) == 0)
		{
			t.type = name.type;
			break;
		}
	}

	return t;
}

inline Token parse_number(Tokenizer* tok)
{
	Token t = {};

	t.type = TOKEN_NUMBER;
	t.start = tok->ptr;
	t.line = tok->line;

	// first character may be a minus
	if (peek(tok) == '-')
	{
		eat(tok);
	}

	bool decimal = false;

	// the first/second character may be the decimal point
	if (peek(tok) == '.')
	{
		decimal = true;

		// bail out early if the next character isn't a number
		if (!is_number(peek(tok, 1))) 
		{
			error(tok, "Unexpected %c", peek(tok));
		}

		eat(tok);
	}

	// start looping
	while (is_number(peek(tok)) || peek(tok) == '.')
	{ 
		if (peek(tok) == '.')
		{
			if (!decimal)
			{
				decimal = true;
			}
			else
			{
				error(tok, "Unexpected `.` in number literal");
			}
		} 

		eat(tok);
	}
	
	t.end = tok->ptr;

	return t;
}

inline Token parse_string(Tokenizer* tok)
{
	Token t = {};

	t.type = TOKEN_STRING;
	t.start = tok->ptr;
	t.line = tok->line;
	
	eat(tok);

	while (eat(tok) != '\'')
	{
		if (!peek(tok))
		{
			error(tok, "Expected `'` to close string literal");
		}
	}

	t.end = tok->ptr;

	return t;
}

void init(Tokenizer* tok, const char* source)
{
	tok->source = source;
	tok->line = 1;
	tok->ptr = 0;
}

void tokenize(Tokenizer* tok, Array <Token>* out)
{
	const size_t source_length = __builtin_strlen(tok->source);

	while (tok->ptr < source_length)
	{
		switch (peek(tok))
		{
			case '\n':
			{
				tok->line++;
			} 
			case '\r':
			case '\t':
			case ' ':
			{
				eat(tok);
			} break;

			case '<':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_LESS_THAN, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_LESS, tok->line);	
				}
			} break;

			case '>':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_GREATER_THAN, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_GREATER, tok->line);	
				}
			} break;

			case '%':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_PERCENT_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_PERCENT, tok->line);		
				}
			} break;

			case '+':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_PLUS_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_PLUS, tok->line);		
				}
			} break;

			case '-':
			{
				if (is_number(peek(tok, 1)) || peek(tok, 1) == '.')
				{
					emplace(out, parse_number(tok));
				}
				else if (peek(tok) == '=')
				{
					eat(tok);
					eat(tok);
					emplace(out, TOKEN_DASH_EQUALS, tok->line);	
				}
				else
				{
					eat(tok);
					emplace(out, TOKEN_DASH, tok->line);		
				}
			} break;

			case '/':
			{
				eat(tok);

				// comment
				if (peek(tok) == '/')
				{
					while (peek(tok) != '\n')
					{
						eat(tok);
					}
				}
				else if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_SLASH_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_SLASH, tok->line);	
				}
			} break;

			case '*':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_STAR_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_STAR, tok->line);		
				}
			} break;

			case '!':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_NOT_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_EXCLAMATION, tok->line);	
				}
			} break;

			case '=':
			{
				eat(tok);

				if (peek(tok) == '=')
				{
					eat(tok);
					emplace(out, TOKEN_DOUBLE_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_EQUALS, tok->line);	
				}
			} break;

			case '&':
			{
				eat(tok);

				if (peek(tok) == '&')
				{
					eat(tok);
					emplace(out, TOKEN_DOUBLE_AND, tok->line);	
				}
			} break;

			case '|':
			{
				eat(tok);

				if (peek(tok) == '|')
				{
					eat(tok);
					emplace(out, TOKEN_DOUBLE_PIPE, tok->line);	
				}
			} break;

			case '{':
			{
				emplace(out, TOKEN_OPEN_BRACE, tok->line);
				eat(tok);
			} break;

			case '}':
			{
				emplace(out, TOKEN_CLOSE_BRACE, tok->line);
				eat(tok);
			} break;

			case '(':
			{
				emplace(out, TOKEN_OPEN_PAREN, tok->line);
				eat(tok);
			} break;

			case ')':
			{
				emplace(out, TOKEN_CLOSE_PAREN, tok->line);		
				eat(tok);
			} break;

			case ',':
			{
				emplace(out, TOKEN_COMMA, tok->line);		
				eat(tok);
			} break;

			case '\'':
			{
				emplace(out, parse_string(tok));
			} break;

			default:
			{
				if (is_alpha(peek(tok)) || peek(tok) == '_')
				{
					emplace(out, parse_identifier(tok));
				}
				// allow the first character to be the decimal point (so numbers like .321 are allowed)
				else if (is_number(peek(tok)) || peek(tok) == '.') 
				{
					emplace(out, parse_number(tok));
				}
				else
				{
					error(tok, "Encountered bad token");
				}
			} break;
		}
	}
}