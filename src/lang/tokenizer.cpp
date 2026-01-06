#include <lang/tokenizer.h>

#include <lang/tokenizer.h>

inline char peek_char(const Tokenizer* tok, int16_t offset = 0) 
{
	if (tok->ptr + offset > __builtin_strlen(tok->source)) 
	{
		return 0;
	}

	return tok->source[tok->ptr + offset];
}

inline char eat_char(Tokenizer* tok)
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

	eat_char(tok);
	
	while (is_alnum(peek_char(tok)) || peek_char(tok) == '_') 
	{ 
		eat_char(tok); 
	}
	
	t.end = tok->ptr;

	// Reassign type if it is a keyword
	const auto equals = [&](const char* name)
	{
		char buf[128] = {};
		as_string(t, tok->source, buf, 128);

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

inline Token parse_number(Tokenizer* tok)
{
	Token t = {};

	t.type = TOKEN_NUMBER;
	t.start = tok->ptr;
	t.line = tok->line;

	bool decimal = false;

	// First character may be the decimal point
	if (peek_char(tok) == '.')
	{
		decimal = true;

		// Bail out early if the next character isn't a number
		if (!is_number(peek_char(tok, 1))) 
		{
			// TODO: Better diagnostics, something like "unexpected `.` near <token>""
			error(tok, "Unexpected `.`");
		}
	}

	eat_char(tok);

	while (is_number(peek_char(tok)) || peek_char(tok) == '.')
	{ 
		if (peek_char(tok) == '.')
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

		eat_char(tok);
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
	
	eat_char(tok);

	while (eat_char(tok) != '\'')
	{
		if (!peek_char(tok))
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
		switch (peek_char(tok))
		{
			case '\n':
			{
				tok->line++;
			} 
			case '\r':
			case '\t':
			case ' ':
			{
				eat_char(tok);
			} break;

			case '<':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_LESS_THAN, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_LESS, tok->line);	
				}
			} break;

			case '>':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_GREATER_THAN, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_GREATER, tok->line);	
				}
			} break;

			case '%':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_PERCENT_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_PERCENT, tok->line);		
				}
			} break;

			case '+':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_PLUS_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_PLUS, tok->line);		
				}
			} break;

			case '-':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_DASH_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_DASH, tok->line);		
				}
			} break;

			case '/':
			{
				eat_char(tok);

				// Comment
				if (peek_char(tok) == '/')
				{
					while (peek_char(tok) != '\n')
					{
						eat_char(tok);
					}
				}
				else if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_SLASH_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_SLASH, tok->line);	
				}
			} break;

			case '*':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_STAR_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_STAR, tok->line);		
				}
			} break;

			case '!':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_NOT_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_EXCLAMATION, tok->line);	
				}
			} break;

			case '=':
			{
				eat_char(tok);

				if (peek_char(tok) == '=')
				{
					eat_char(tok);
					emplace(out, TOKEN_DOUBLE_EQUALS, tok->line);	
				}
				else
				{
					emplace(out, TOKEN_EQUALS, tok->line);	
				}
			} break;

			case '&':
			{
				if (peek_char(tok) == '&')
				{
					eat_char(tok);
					emplace(out, TOKEN_DOUBLE_AND, tok->line);	
				}
			} break;

			case '|':
			{
				if (peek_char(tok) == '|')
				{
					eat_char(tok);
					emplace(out, TOKEN_DOUBLE_PIPE, tok->line);	
				}
			} break;

			case '{':
			{
				emplace(out, TOKEN_OPEN_CURLY, tok->line);
				eat_char(tok);
			} break;

			case '}':
			{
				emplace(out, TOKEN_CLOSE_CURLY, tok->line);
				eat_char(tok);
			} break;

			case '(':
			{
				emplace(out, TOKEN_OPEN_BRACE, tok->line);
				eat_char(tok);
			} break;

			case ')':
			{
				emplace(out, TOKEN_CLOSE_BRACE, tok->line);		
				eat_char(tok);
			} break;

			case ',':
			{
				emplace(out, TOKEN_COMMA, tok->line);		
				eat_char(tok);
			} break;

			case '\'':
			{
				emplace(out, parse_string(tok));
			} break;

			default:
			{
				if (is_alpha(peek_char(tok)) || peek_char(tok) == '_')
				{
					emplace(out, parse_identifier(tok));
				}
				// Allow the first character to be the decimal point (so numbers like .321 are allowed)
				else if (is_number(peek_char(tok)) || peek_char(tok) == '.') 
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