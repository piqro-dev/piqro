#include <pq/compiler.h>

#define C_ERROR(...) \
	do \
	{ \
		char what[2048]; \
		sprintf(what, __VA_ARGS__); \
		\
		c->error(c->line, what); \
	} while (0);

//
// tokenization
//

static char peek_char(PQ_Compiler* c, int16_t offset)
{
	if (c->idx + offset >= c->source.length) 
	{ 
		return 0; 
	} 
	else 
	{ 
		return c->source.buffer[c->idx + offset]; 
	} 
}

static char eat_char(PQ_Compiler* c)
{
	return c->source.buffer[c->idx++];
}

static void push_token(PQ_Compiler* c, PQ_Token t)
{
	c->tokens[c->token_count++] = t;
}

static constexpr struct 
{
	PQ_TokenType type;
	String name;
} KEYWORDS[] =
{
	{ TOKEN_NULL,    s("null") },
	{ TOKEN_TRUE,    s("true") },
	{ TOKEN_FALSE,   s("false") },
	{ TOKEN_VAR,     s("var") },
	{ TOKEN_FOREVER, s("forever") },
	{ TOKEN_REPEAT,  s("repeat") },
	{ TOKEN_UNTIL,   s("until") },
	{ TOKEN_DEFINE,  s("define") },
	{ TOKEN_RETURN,  s("return") },
	{ TOKEN_IF,      s("if") },
	{ TOKEN_ELSE,    s("else") },
	{ TOKEN_BREAK,   s("break") },
	{ TOKEN_FOREIGN, s("foreign") },
};

static PQ_Token parse_identifier(PQ_Compiler* c)
{
	PQ_Token t = {};
	
	t.type = TOKEN_IDENTIFIER;
	t.start = c->idx;
	t.line = c->line;

	eat_char(c);
	
	while (is_alnum(peek_char(c, 0)) || peek_char(c, 0) == '_') 
	{ 
		eat_char(c); 
	}
	
	t.end = c->idx;

	// reassign type if it is a keyword
	for (size_t i = 0; i < COUNT_OF(KEYWORDS); i++)
	{
		String name = pq_token_as_string(c->arena, t, c->source);

		if (str_equals(name, KEYWORDS[i].name))
		{
			t.type = KEYWORDS[i].type;
			break;
		}
	}

	return t;
}

static PQ_Token parse_number(PQ_Compiler* c)
{
	PQ_Token t = {};

	t.type = TOKEN_NUMBER;
	t.start = c->idx;
	t.line = c->line;

	// first character may be a minus
	if (peek_char(c, 0) == '-')
	{
		eat_char(c);
	}

	// hexadecimal
	if (peek_char(c, 0) == '0' && (peek_char(c, 1) == 'X' || peek_char(c, 1) == 'x'))
	{
		eat_char(c);
		eat_char(c);

		while (is_alnum(peek_char(c, 0)))
		{
			eat_char(c);
		}
	}
	// decimal
	else
	{		
		bool decimal = false;
	
		// the first/second character may be the decimal point
		if (peek_char(c, 0) == '.')
		{
			decimal = true;
	
			// bail out early if the next character isn't a number
			if (!is_number(peek_char(c, 1))) 
			{
				C_ERROR("Unexpected %c", peek_char(c, 0));
			}
	
			eat_char(c);
		}
	
		// start looping
		while (is_number(peek_char(c, 0)) || peek_char(c, 0) == '.')
		{ 
			if (peek_char(c, 0) == '.')
			{
				if (!decimal)
				{
					decimal = true;
				}
				else
				{
					C_ERROR("Unexpected `.` in number literal");
				}
			} 
	
			eat_char(c);
		}
	}
	
	t.end = c->idx;

	return t;
}

static PQ_Token parse_string(PQ_Compiler* c)
{
	PQ_Token t = {};

	t.type = TOKEN_STRING;
	t.start = c->idx;
	t.line = c->line;
	
	eat_char(c);

	while (eat_char(c) != '\'')
	{
		if (!peek_char(c, 0) || peek_char(c, 0) == '\n')
		{
			C_ERROR("Expected `'` to close string literal");
		}
	}

	t.end = c->idx;

	return t;
}

static void tokenize(PQ_Compiler* c)
{
	c->idx = 0;

	while (c->idx < c->source.length)
	{
		switch (peek_char(c, 0))
		{
			case '\n':
			{
				c->line++;
			} 
			case '\r':
			case '\t':
			case ' ':
			{
				eat_char(c);
			} break;

			case '<':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_LESS_THAN, c->line });	
				}
				else if (peek_char(c, 0) == '<')
				{
					eat_char(c);

					if (peek_char(c, 0) == '=')
					{
						eat_char(c);
						push_token(c, (PQ_Token){ TOKEN_LEFT_SHIFT_EQUALS, c->line });	
					}
					else
					{
						push_token(c, (PQ_Token){ TOKEN_LEFT_SHIFT, c->line });	
					}
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_LESS, c->line });	
				}
			} break;

			case '>':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_GREATER_THAN, c->line });	
				}
				else if (peek_char(c, 0) == '>')
				{
					eat_char(c);
				
					if (peek_char(c, 0) == '=')
					{
						eat_char(c);
						push_token(c, (PQ_Token){ TOKEN_RIGHT_SHIFT_EQUALS, c->line });	
					}
					else
					{
						push_token(c, (PQ_Token){ TOKEN_RIGHT_SHIFT, c->line });	
					}
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_GREATER, c->line });	
				}
			} break;

			case '%':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_PERCENT_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_PERCENT, c->line });		
				}
			} break;

			case '+':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_PLUS_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_PLUS, c->line });		
				}
			} break;

			case '-':
			{
				if (is_number(peek_char(c, 1)) || peek_char(c, 1) == '.')
				{
					push_token(c, parse_number(c));
				}
				else if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					eat_char(c);

					push_token(c, (PQ_Token){ TOKEN_DASH_EQUALS, c->line });	
				}
				else
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_DASH, c->line });		
				}
			} break;

			case '/':
			{
				eat_char(c);

				if (peek_char(c, 0) == '/')
				{
					while (peek_char(c, 0) != '\n' && peek_char(c, 0))
					{
						eat_char(c);
					}
				}
				else if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_SLASH_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_SLASH, c->line });	
				}
			} break;

			case '*':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_STAR_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_STAR, c->line });		
				}
			} break;

			case '!':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_NOT_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_EXCLAMATION, c->line });	
				}
			} break;

			case '=':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_DOUBLE_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_EQUALS, c->line });	
				}
			} break;

			case '&':
			{
				eat_char(c);

				if (peek_char(c, 0) == '&')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_DOUBLE_AND, c->line });	
				}
				else if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_AND_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_AND, c->line });	
				}
			} break;

			case '|':
			{
				eat_char(c);

				if (peek_char(c, 0) == '|')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_DOUBLE_PIPE, c->line });	
				}
				else if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_PIPE_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_PIPE, c->line });	
				}
			} break;

			case '^':
			{
				eat_char(c);

				if (peek_char(c, 0) == '=')
				{
					eat_char(c);
					push_token(c, (PQ_Token){ TOKEN_CARET_EQUALS, c->line });	
				}
				else
				{
					push_token(c, (PQ_Token){ TOKEN_CARET, c->line });	
				}
			} break;

			case '{':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_OPEN_BRACE, c->line });
			} break;

			case '}':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_CLOSE_BRACE, c->line });
			} break;

			case '(':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_OPEN_PAREN, c->line });
			} break;

			case ']':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_CLOSE_BOX, c->line });
			} break;

			case '[':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_OPEN_BOX, c->line });
			} break;

			case ')':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_CLOSE_PAREN, c->line });		
			} break;

			case ',':
			{
				eat_char(c);
				push_token(c, (PQ_Token){ TOKEN_COMMA, c->line });		
			} break;

			case '\'':
			{
				push_token(c, parse_string(c));
			} break;

			// semicolons aren't mandatory, it's fine
			case ';':
			{
				eat_char(c);
			} break;

			default:
			{
				if (is_alpha(peek_char(c, 0)) || peek_char(c, 0) == '_')
				{
					push_token(c, parse_identifier(c));
				}
				// allow the first character to be the decimal point (so numbers like .321 are allowed)
				else if (is_number(peek_char(c, 0)) || peek_char(c, 0) == '.') 
				{
					push_token(c, parse_number(c));
				}
				else
				{
					C_ERROR("Encountered bad token");
				}
			} break;
		}
	}
}

//
// code generation
//

static PQ_Token peek_token(PQ_Compiler* c, int16_t offset)
{
	if (c->idx + offset > c->token_count) 
	{ 
		return (PQ_Token){ TOKEN_UNKNOWN }; 
	} 
	else 
	{ 
		return c->tokens[c->idx + offset]; 
	} 
}

static PQ_Token eat_token(PQ_Compiler* c)
{
	PQ_Token t = c->tokens[c->idx++];

	c->line = t.line;

	return t;
}

static PQ_Token try_eat_token(PQ_Compiler* c, PQ_TokenType expected)
{
	PQ_TokenType next = peek_token(c, 0).type;

	if (next != expected)
	{
		C_ERROR("Expected %s, got %s", pq_token_to_c_str(expected), pq_token_to_c_str(next));
	}

	return eat_token(c);
}

static PQ_Instruction* push_inst(PQ_Compiler* c, PQ_Instruction it)
{
	c->instructions[c->instruction_count++] = it;

	return &c->instructions[c->instruction_count - 1];
}

static bool variable_exists(PQ_Compiler* c, String name)
{
	for (uint16_t i = 0; i < c->variable_count; i++)
	{
		if (str_equals(c->variables[i].name, name))
		{
			return true;
		}
	}

	return false;
}

static bool procedure_exists(PQ_Compiler* c, String name)
{
	for (uint16_t i = 0; i < c->procedure_count; i++)
	{
		if (str_equals(c->procedures[i].name, name))
		{
			return true;
		}
	}

	return false;
}

static PQ_Procedure* get_or_create_procedure(PQ_Compiler* c, String name)
{
	for (uint16_t i = 0; i < c->procedure_count; i++)
	{
		if (str_equals(c->procedures[i].name, name))
		{
			return &c->procedures[i];
		}
	}

	PQ_Procedure* proc = &c->procedures[c->procedure_count++];
	
	proc->name = str_copy(c->arena, name);
	proc->idx = c->procedure_count - 1;

	return proc;
}

static PQ_Variable* get_or_create_variable(PQ_Compiler* c, String name)
{
	for (uint16_t i = 0; i < c->variable_count; i++)
	{
		if (str_equals(c->variables[i].name, name))
		{
			return &c->variables[i];
		}
	}

	PQ_Variable* var = &c->variables[c->variable_count++];

	var->name = str_copy(c->arena, name); 
	var->idx = c->variable_count - 1;
	
	var->arg = false;
	var->array = false;
	var->global = false;

	if (!c->current_proc && !c->current_scope)
	{
		var->global = true;
	}

	return var;
}

static uint16_t get_or_create_immediate(PQ_Compiler* c, PQ_Value v)
{
	for (uint16_t i = 0; i < c->immediate_count; i++)
	{
		if (pq_value_equals(c->immediates[i], v).b)
		{
			return i;
		}
	}

	c->immediates[c->immediate_count++] = v;

	return c->immediate_count - 1;
}

static void emit_expression(PQ_Compiler* c);

static void emit_statement(PQ_Compiler* c);

//
// expressions
//

static void emit_null_expression(PQ_Compiler* c)
{
	PQ_Token null = eat_token(c);

	push_inst(c, (PQ_Instruction){ INST_LOAD_NULL });
}

static void emit_number_expression(PQ_Compiler* c)
{
	PQ_Token number = eat_token(c);

	PQ_Value imm = pq_value_number(str_as_number(str_copy_from_to(c->arena, c->source, number.start, number.end)));

	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, imm) });
}

static void emit_identifier_expression(PQ_Compiler* c)
{
	PQ_Token ident = eat_token(c);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if (!variable_exists(c, name))
	{
		C_ERROR("Undefined identifier '%.*s'", s_fmt(name));
	}

	PQ_Variable* var = get_or_create_variable(c, name);

	push_inst(c, (PQ_Instruction){ var->global ? INST_LOAD_GLOBAL : INST_LOAD_LOCAL, var->idx });
}

static void emit_boolean_expression(PQ_Compiler* c)
{
	PQ_Token boolean = eat_token(c);

	bool r = false;

	switch (boolean.type)
	{
		case TOKEN_TRUE:  r = true; break;
		case TOKEN_FALSE: r = false; break;

		default: break;
	}

	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, pq_value_boolean(r)) });
}

// <ident>(<expr>, <expr>...)
static void emit_procedure_expression(PQ_Compiler* c)
{
	PQ_Token ident = eat_token(c);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if (!procedure_exists(c, name))
	{
		C_ERROR("Undefined procedure '%.*s'", s_fmt(name));
	}

	// (
	eat_token(c);

	uint8_t arg_count = 0; 

	// <expr>, <expr>...
	while (peek_token(c, 0).type != TOKEN_CLOSE_PAREN)
	{
		// check for (,<expr>
		if (peek_token(c, -1).type == TOKEN_OPEN_PAREN && peek_token(c, 0).type == TOKEN_COMMA)
		{
			C_ERROR("Expected expression after `(` in procedure call, got `,`");
		}

		// <expr>
		emit_expression(c);

		arg_count++;

		if (peek_token(c, 0).type != TOKEN_CLOSE_PAREN)
		{
			// ,
			try_eat_token(c, TOKEN_COMMA);
	
			// check for <expr>,)
			if (peek_token(c, 0).type == TOKEN_CLOSE_PAREN)
			{
				C_ERROR("Expected expression after `,` in procedure call, got `)`");
			}
		}
	}

	PQ_Procedure* proc = get_or_create_procedure(c, name);

	if (proc->arg_count != arg_count)
	{
		C_ERROR("Expected %d arguments for procedure '%.*s', got %d", proc->arg_count, s_fmt(name), arg_count);
	}

	// )
	try_eat_token(c, TOKEN_CLOSE_PAREN);

	// call the procedure
	push_inst(c, (PQ_Instruction){ INST_CALL, proc->idx });
}

// <ident> =... <expr>
static void emit_assign_expression(PQ_Compiler* c) 
{
	// <ident> 
	PQ_Token ident = eat_token(c);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if (!variable_exists(c, name)) 
	{
		C_ERROR("Undefined identifier '%.*s'", s_fmt(name));
	}

	PQ_Variable* var = get_or_create_variable(c, name);

	// =...
	PQ_Token assign = eat_token(c);

	// when we are just assigning, we don't operate on the identifier
	if (assign.type != TOKEN_EQUALS)
	{
		push_inst(c, (PQ_Instruction){ var->global ? INST_LOAD_GLOBAL : INST_LOAD_LOCAL, var->idx });
	}

	emit_expression(c);

	switch (assign.type)
	{
		case TOKEN_EQUALS: break;

		case TOKEN_PLUS_EQUALS:    push_inst(c, (PQ_Instruction){ INST_ADD }); break;
		case TOKEN_DASH_EQUALS:    push_inst(c, (PQ_Instruction){ INST_SUB }); break;
		case TOKEN_SLASH_EQUALS:   push_inst(c, (PQ_Instruction){ INST_DIV }); break;
		case TOKEN_STAR_EQUALS:    push_inst(c, (PQ_Instruction){ INST_MUL }); break;
		case TOKEN_PERCENT_EQUALS: push_inst(c, (PQ_Instruction){ INST_MOD }); break;

		default: C_ERROR("Unexpected %s", pq_token_to_c_str(assign.type)); break;
	}

	push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL : INST_STORE_LOCAL, var->idx });
}

// <expr> <op> <expr>
// based on https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
static void emit_binary_expression(PQ_Compiler* c, int8_t min_precedence)
{
	// avoid recursion
	if (min_precedence > -1) 
	{
		// left <expr>
		emit_expression(c);
	}

	// <op>
	PQ_Token op = eat_token(c);

	// right <expr>
	emit_expression(c);

	switch (op.type)
	{
		case TOKEN_PERCENT:       push_inst(c, (PQ_Instruction){ INST_MOD }); break; 
		case TOKEN_PLUS:          push_inst(c, (PQ_Instruction){ INST_ADD }); break; 
		case TOKEN_DASH:          push_inst(c, (PQ_Instruction){ INST_SUB }); break;
		case TOKEN_SLASH:         push_inst(c, (PQ_Instruction){ INST_DIV }); break; 
		case TOKEN_STAR:          push_inst(c, (PQ_Instruction){ INST_MUL }); break; 

		case TOKEN_LEFT_SHIFT:    push_inst(c, (PQ_Instruction){ INST_BW_LEFT_SHIFT }); break;  
		case TOKEN_RIGHT_SHIFT:   push_inst(c, (PQ_Instruction){ INST_BW_RIGHT_SHIFT }); break;  

		case TOKEN_LESS:          push_inst(c, (PQ_Instruction){ INST_LESS }); break; 
		case TOKEN_GREATER:       push_inst(c, (PQ_Instruction){ INST_GREATER }); break; 
		case TOKEN_LESS_THAN:     push_inst(c, (PQ_Instruction){ INST_LESS_THAN }); break; 
		case TOKEN_GREATER_THAN:  push_inst(c, (PQ_Instruction){ INST_GREATER_THAN }); break; 
		
		case TOKEN_DOUBLE_EQUALS: push_inst(c, (PQ_Instruction){ INST_EQUALS }); break;
		case TOKEN_NOT_EQUALS:    push_inst(c, (PQ_Instruction){ INST_EQUALS }); push_inst(c, (PQ_Instruction){ INST_NOT }); break; 
		
		case TOKEN_PIPE:          push_inst(c, (PQ_Instruction){ INST_BW_OR }); break;
		case TOKEN_CARET:         push_inst(c, (PQ_Instruction){ INST_BW_XOR }); break;
		case TOKEN_AND:           push_inst(c, (PQ_Instruction){ INST_BW_AND }); break;

		case TOKEN_DOUBLE_AND:    push_inst(c, (PQ_Instruction){ INST_AND }); break; 
		case TOKEN_DOUBLE_PIPE:   push_inst(c, (PQ_Instruction){ INST_OR }); break; 
		
		default: C_ERROR("Expected binary operator, got %s", pq_token_to_c_str(op.type)); 
	}

	// next token
	op = peek_token(c, 0);

	while (pq_token_is_binary_op(op.type) && pq_token_precedence_of(op.type) >= min_precedence)
	{
		min_precedence = pq_token_precedence_of(op.type) + 1;
		emit_binary_expression(c, min_precedence);
	}
}

static void emit_string_expression(PQ_Compiler* c)
{
	PQ_Token str = eat_token(c);

	PQ_Value imm = pq_value_string(str_copy_from_to(c->arena, c->source, str.start, str.end));

	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, imm) });
}

// <ident>[<expr>] 
// OR
// <ident>[<expr>] = <expr>
static void emit_array_element_expression(PQ_Compiler* c)
{
	uint16_t start_pos = c->idx;
	
	// <ident>
	PQ_Token ident = eat_token(c);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if (!variable_exists(c, name))
	{
		C_ERROR("Undefined identifier '%.*s'", s_fmt(name));
	}

	PQ_Variable* var = get_or_create_variable(c, name);

	if (!var->array && !var->arg)
	{
		C_ERROR("Tried indexing non-array identifier '%.*s'", s_fmt(name));
	}

	uint16_t start = c->instruction_count;
	
	// [
	eat_token(c);

	// <expr>
	emit_expression(c);
	
	// ]
	try_eat_token(c, TOKEN_CLOSE_BOX);

	push_inst(c, (PQ_Instruction){ var->global ? INST_LOAD_GLOBAL_SUBSCRIPT : INST_LOAD_LOCAL_SUBSCRIPT, var->idx });	

	// ..=..
	if (pq_token_is_assign_op(peek_token(c, 0).type))
	{
		PQ_Token assign = eat_token(c);

		// when we are just assigning, we don't operate on the identifier
		if (assign.type == TOKEN_EQUALS)
		{
			// pop the array element off
			c->instruction_count = start;
		}

		emit_expression(c);

		switch (assign.type)
		{
			case TOKEN_EQUALS: break;
	
			case TOKEN_PLUS_EQUALS:        push_inst(c, (PQ_Instruction){ INST_ADD }); break;
			case TOKEN_DASH_EQUALS:        push_inst(c, (PQ_Instruction){ INST_SUB }); break;
			case TOKEN_SLASH_EQUALS:       push_inst(c, (PQ_Instruction){ INST_DIV }); break;
			case TOKEN_STAR_EQUALS:        push_inst(c, (PQ_Instruction){ INST_MUL }); break;
			case TOKEN_PERCENT_EQUALS:     push_inst(c, (PQ_Instruction){ INST_MOD }); break;
	
			case TOKEN_LEFT_SHIFT_EQUALS:  push_inst(c, (PQ_Instruction){ INST_BW_LEFT_SHIFT }); break; 
			case TOKEN_RIGHT_SHIFT_EQUALS: push_inst(c, (PQ_Instruction){ INST_BW_RIGHT_SHIFT }); break;

			case TOKEN_PIPE_EQUALS:        push_inst(c, (PQ_Instruction){ INST_BW_OR }); break;
			case TOKEN_CARET_EQUALS:       push_inst(c, (PQ_Instruction){ INST_BW_XOR }); break;
			case TOKEN_AND_EQUALS:         push_inst(c, (PQ_Instruction){ INST_BW_AND }); break;

			default: C_ERROR("Unexpected %s", pq_token_to_c_str(assign.type)); break;
		}

		uint16_t current_pos = c->idx;

		// jump back to the start and read out the subscript
		c->idx = start_pos + 1; // skip identifier

		// [
		eat_token(c);

		// <expr>
		emit_expression(c);

		// ]
		eat_token(c);

		// ...go back where we left off
		c->idx = current_pos;

		push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL_SUBSCRIPT : INST_STORE_LOCAL_SUBSCRIPT, var->idx });
	}
}

static void emit_expression(PQ_Compiler* c)
{
	PQ_Token expr = peek_token(c, 0);

	switch (expr.type)
	{
		case TOKEN_NULL:
		{
			emit_null_expression(c);
		} break;

		case TOKEN_DASH:
		{
			eat_token(c);
			emit_expression(c);
			push_inst(c, (PQ_Instruction){ INST_NEGATE });
		} break;

		case TOKEN_OPEN_PAREN:
		{
			eat_token(c);
			emit_expression(c);
			try_eat_token(c, TOKEN_CLOSE_PAREN);
		} break;

		case TOKEN_NUMBER:
		{
			emit_number_expression(c);
		} break;

		case TOKEN_STRING:
		{
			emit_string_expression(c);
		} break;

		case TOKEN_IDENTIFIER:
		{
			if (peek_token(c, 1).type == TOKEN_OPEN_BOX)
			{
				emit_array_element_expression(c);
			}
			else if (peek_token(c, 1).type == TOKEN_OPEN_PAREN)
			{
				emit_procedure_expression(c);
			}
			else if (pq_token_is_assign_op(peek_token(c, 1).type))
			{
				emit_assign_expression(c);
			}
			else
			{
				emit_identifier_expression(c);
			}
		} break;

		case TOKEN_TRUE ... TOKEN_FALSE:
		{
			emit_boolean_expression(c);
		} break;

		default:
		{
			C_ERROR("Expected expression, got %s", pq_token_to_c_str(expr.type));
		} break;
	}

	if (pq_token_is_binary_op(peek_token(c, 0).type))
	{
		emit_binary_expression(c, -1);
	}
}

//
// scope
//

static void begin_scope(PQ_Compiler* c, PQ_Scope* scope) 
{
	// {
	try_eat_token(c, TOKEN_OPEN_BRACE);

	scope->first_inst = c->instruction_count;
	scope->local_base = c->variable_count;

	c->current_scope = scope;
}

static void end_scope(PQ_Compiler* c, PQ_Scope* scope) 
{
	// }
	try_eat_token(c, TOKEN_CLOSE_BRACE);

	scope->last_inst = c->instruction_count;

	c->variable_count = scope->local_base;

	c->current_scope = nullptr;
}

static void emit_scope(PQ_Compiler* c)
{
	PQ_Scope scope = {};

	begin_scope(c, &scope);

	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	end_scope(c, &scope);
}

//
// statements
//

// var <ident> 
// OR 
// var <ident> = <expr>
// OR
// var <ident>[N] | var <ident>[] = { ... } | var <ident>[N] = { ... }
static void emit_var_statement(PQ_Compiler* c)
{
	// var
	eat_token(c);
	
	// <ident>
	PQ_Token ident = try_eat_token(c, TOKEN_IDENTIFIER);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if ((variable_exists(c, name) || procedure_exists(c, name))) 
	{
		C_ERROR("Identifier '%.*s' redefined", s_fmt(name));
	}

	PQ_Variable* var = get_or_create_variable(c, name);

	// variables local to procedures get their index redefined
	if (c->current_proc && c->current_scope == &c->current_proc->scope) 
	{
		var->idx = c->current_proc->arg_count + c->current_proc->local_count++;
	}

	// [
	if (peek_token(c, 0).type == TOKEN_OPEN_BOX)
	{
		eat_token(c);

		var->array = true;

		// unknown size
		if (peek_token(c, 0).type == TOKEN_CLOSE_BOX)
		{
			var->array_size = (uint16_t)-1;
		}
		// not a number
		else if (peek_token(c, 0).type != TOKEN_NUMBER)
		{
			C_ERROR("Expected number literal as array size, got %s", pq_token_to_c_str(peek_token(c, 0).type));
		}
		else
		{
			PQ_Token number = eat_token(c);
	
			int32_t N = (int32_t)str_as_number(str_copy_from_to(c->arena, c->source, number.start, number.end));
			
			if (N <= 0)
			{
				C_ERROR("Size of array '%.*s' cannot be negative or zero", s_fmt(var->name));
			}
	
			var->array_size = (uint16_t)N;
		}

		// ]
		try_eat_token(c, TOKEN_CLOSE_BOX);
	}

	// =
	if (peek_token(c, 0).type == TOKEN_EQUALS)
	{
		eat_token(c);

		// initializer list
		if (var->array)
		{
			PQ_Instruction* load_array = push_inst(c, (PQ_Instruction){ INST_LOAD_ARRAY });
			push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL : INST_STORE_LOCAL, var->idx });

			uint16_t size = 0;

			// {
			try_eat_token(c, TOKEN_OPEN_BRACE);

			// ...
			while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
			{
				// check for { ,<expr>
				if (peek_token(c, -1).type == TOKEN_OPEN_BRACE && peek_token(c, 0).type == TOKEN_COMMA)
				{
					C_ERROR("Expected expression after `{` in initializer list, got `,`");
				}

				// <expr>
				emit_expression(c);

				push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, pq_value_number(size++)) });

				push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL_SUBSCRIPT : INST_STORE_LOCAL_SUBSCRIPT, var->idx });

				if (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
				{
					// ,
					try_eat_token(c, TOKEN_COMMA);
				
					// check for <expr>, }
					if (peek_token(c, 0).type == TOKEN_CLOSE_BRACE)
					{
						C_ERROR("Expected expression after `,` in initializer list, got `}`");
					}
				}
			}

			// }
			try_eat_token(c, TOKEN_CLOSE_BRACE);

			if (var->array_size == (uint16_t)-1)
			{
				if (size == 0)
				{
					C_ERROR("Initializer list of unknown sized array cannot be empty");
				}

				var->array_size = size;
			}
			else if (size > var->array_size)
			{
				C_ERROR("Initializer list of array is bigger than it's size")
			}

			load_array->arg = var->array_size;
		}
		else
		{
			emit_expression(c);

			push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL : INST_STORE_LOCAL, var->idx });
		}
	}
	// declaration
	else
	{
		// array
		if (var->array)
		{
			if (var->array_size == (uint16_t)-1)
			{
				C_ERROR("Declaration of unknown sized array is not allowed");
			}

			push_inst(c, (PQ_Instruction){ INST_LOAD_ARRAY, var->array_size });
			push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL : INST_STORE_LOCAL, var->idx });
		}
		// simple variable
		else
		{
			push_inst(c, (PQ_Instruction){ INST_LOAD_NULL });
			push_inst(c, (PQ_Instruction){ var->global ? INST_STORE_GLOBAL : INST_STORE_LOCAL, var->idx });
		}
	}
}

// define <ident>(<ident>, <ident>...) { ... }
static void emit_define_statement(PQ_Compiler* c)
{
	// define
	eat_token(c);

	// <ident>
	PQ_Token ident = try_eat_token(c, TOKEN_IDENTIFIER);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if (variable_exists(c, name) || procedure_exists(c, name)) 
	{
		C_ERROR("Identifier '%.*s' redefined", s_fmt(name));
	}

	if (c->current_proc)
	{
		C_ERROR("Cannot define procedure inside another one");
	}

	PQ_Procedure* proc = get_or_create_procedure(c, name);

	c->current_proc = proc;

	// (
	PQ_Token open = try_eat_token(c, TOKEN_OPEN_PAREN);

	// <ident>, <ident>...
	while (peek_token(c, 0).type != TOKEN_CLOSE_PAREN)
	{
		// check for (,<ident>
		if (peek_token(c, -1).type == TOKEN_OPEN_PAREN && peek_token(c, 0).type == TOKEN_COMMA)
		{
			C_ERROR("Expected argument after `(` in procedure definition, got `,`");
		}

		// <ident>
		PQ_Token ident = try_eat_token(c, TOKEN_IDENTIFIER);

		String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

		if (variable_exists(c, name))
		{
			C_ERROR("Argument/variable redefinition in procedure definition");
		}

		PQ_Variable* var = get_or_create_variable(c, name);

		var->idx = proc->arg_count++;
		var->arg = true;

		if (peek_token(c, 0).type != TOKEN_CLOSE_PAREN)
		{
			// ,
			try_eat_token(c, TOKEN_COMMA);
		
			// check for <ident>,)
			if (peek_token(c, 0).type == TOKEN_CLOSE_PAREN)
			{
				C_ERROR("Expected argument after `,` in procedure definition, got `)`");
			}
		}
	}

	// )
	try_eat_token(c, TOKEN_CLOSE_PAREN);

	// skip the function body
	PQ_Instruction* jump = push_inst(c, (PQ_Instruction){ INST_JUMP });

	// {
	begin_scope(c, &proc->scope);

	// move the local index back to the first argument
	proc->scope.local_base -= proc->arg_count;

	// { ... }
	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	// regardless of returning anything, all procedures will
	// return `null`. a return statement beforehand would circumvent these
	// instructions. 
	if (c->instructions[c->instruction_count - 1].type != INST_RETURN)
	{
		push_inst(c, (PQ_Instruction){ INST_LOAD_NULL });
		push_inst(c, (PQ_Instruction){ INST_RETURN });
	}
	
	// }
	end_scope(c, &proc->scope);

	c->current_proc = nullptr;

	// we know the last instruction now, we'll jump here.
	jump->arg = proc->scope.last_inst;
}

// return <expr>
static void emit_return_statement(PQ_Compiler* c)
{
	if (!c->current_proc)
	{
		C_ERROR("Cannot use return outside procedure");
	}

	// return
	eat_token(c);

	// <expr>
	emit_expression(c);

	push_inst(c, (PQ_Instruction){ INST_RETURN });
}

static void patch_breaks(PQ_Compiler* c, const PQ_Loop* loop) 
{
	for (uint16_t i = loop->scope.first_inst; i < loop->scope.last_inst; i++) 
	{
		PQ_Instruction* it = &c->instructions[i];

		if (it->type == INST_JUMP && it->arg == (uint16_t)-1) 
		{
			it->arg = loop->scope.last_inst + 1;
		}
	}
}

//
// the statement `repeat <expr> { ... }` replicates the equivalent of this C code:
//
// int repeat_local = <expr>;
//
// while (--repeat_local >= 0) 
// {
//   <...>
// }
// 
static void emit_repeat_statement(PQ_Compiler* c)
{
	// repeat
	eat_token(c);

	// <expr>
	emit_expression(c);

	PQ_Loop loop = {};
	
	// {
	begin_scope(c, &loop.scope);

	String name = str_format(c->arena, "__repeat_local_%d", loop.scope.local_base);

	loop.local = get_or_create_variable(c, name)->idx;

	// repeat_local = <expr>
	push_inst(c, (PQ_Instruction){ INST_STORE_LOCAL, loop.local });

	// repeat_local >= 0
	push_inst(c, (PQ_Instruction){ INST_LOAD_LOCAL, loop.local });
	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, pq_value_number(0.0f)) });
	push_inst(c, (PQ_Instruction){ INST_LESS_THAN });

	// true? skip the loop
	PQ_Instruction* jump_cond = push_inst(c, (PQ_Instruction){ INST_JUMP_COND });

	// repeat_local = repeat_local - 1
	push_inst(c, (PQ_Instruction){ INST_LOAD_LOCAL, loop.local });
	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, pq_value_number(1.0f)) });
	push_inst(c, (PQ_Instruction){ INST_SUB });
	push_inst(c, (PQ_Instruction){ INST_STORE_LOCAL, loop.local });

	c->current_loop = &loop;

	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	c->current_loop = nullptr;

	end_scope(c, &loop.scope);

	push_inst(c, (PQ_Instruction){ INST_JUMP, loop.scope.first_inst + 1 });

	jump_cond->arg = loop.scope.last_inst + 1; // last statement + the jump

	// patch breaks
	patch_breaks(c, &loop);
}

// repeat until <expr> { ... }
static void emit_repeat_until_statement(PQ_Compiler* c) 
{
	// repeat
	eat_token(c);

	// until
	eat_token(c);

	PQ_Loop loop = {};

	uint16_t start = c->instruction_count;

	// <expr>
	emit_expression(c);
	
	// {
	begin_scope(c, &loop.scope);

	loop.scope.first_inst = start;

	// is <expr> true? Skip the loop
	PQ_Instruction* jump_cond = push_inst(c, (PQ_Instruction){ INST_JUMP_COND });

	c->current_loop = &loop;

	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	c->current_loop = nullptr;

	end_scope(c, &loop.scope);

	push_inst(c, (PQ_Instruction){ INST_JUMP, loop.scope.first_inst });

	jump_cond->arg = loop.scope.last_inst + 1; // last statement + the jump

	// patch breaks
	patch_breaks(c, &loop);
}

// forever { ... }
static void emit_forever_statement(PQ_Compiler* c)
{
	// forever
	eat_token(c);	

	PQ_Loop loop = {};

	// {
	begin_scope(c, &loop.scope);
	
	c->current_loop = &loop;

	// statements
	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	c->current_loop = nullptr;

	// }
	end_scope(c, &loop.scope);

	push_inst(c, (PQ_Instruction){ INST_JUMP, loop.scope.first_inst });

	patch_breaks(c, &loop);
}

static void emit_else_if_statement(PQ_Compiler* c);

static void emit_else_statement(PQ_Compiler* c);

// if <expr> { ... }
static void emit_if_statement(PQ_Compiler* c)
{
	// if
	eat_token(c);

	// <expr>
	emit_expression(c);

	// <expr> == false?
	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, pq_value_number(false)) });
	push_inst(c, (PQ_Instruction){ INST_EQUALS });

	// true? skip the block.
	PQ_Instruction* jump_cond = push_inst(c, (PQ_Instruction){ INST_JUMP_COND });

	PQ_Scope scope = {};

	// {
	begin_scope(c, &scope);

	// Statements
	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}
 
	// }
	end_scope(c, &scope);

	// this jumps to the end of this block if <expr> == false (therefore, not executing this block).
	jump_cond->arg = scope.last_inst;

	if (peek_token(c, 0).type == TOKEN_ELSE)
	{
		// these emit an additional jump at the beginning, in case this block ends up executing, so jump here instead.
		if (peek_token(c, 1).type == TOKEN_IF)
		{
			emit_else_if_statement(c);
			jump_cond->arg++;
		}
		else
		{
			emit_else_statement(c);
			jump_cond->arg++;
		}
	}
}

// else if <expr> { ... }
static void emit_else_if_statement(PQ_Compiler* c) 
{
	// if the last block executes, this will skip this block.
	PQ_Instruction* jump = push_inst(c, (PQ_Instruction){ INST_JUMP });

	// else if
	eat_token(c);
	eat_token(c);

	// <expr>
	emit_expression(c);

	// <expr> == false?
	push_inst(c, (PQ_Instruction){ INST_LOAD_IMMEDIATE, get_or_create_immediate(c, pq_value_number(false)) });
	push_inst(c, (PQ_Instruction){ INST_EQUALS });

	// true? skip the block.
	PQ_Instruction* jump_cond = push_inst(c, (PQ_Instruction){ INST_JUMP_COND });

	PQ_Scope scope = {};

	// {
	begin_scope(c, &scope);

	// Statements
	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	// }
	end_scope(c, &scope);

	// this jumps to the end of this block if the last block was executed. 
	jump->arg = scope.last_inst;

	// this jumps to the end of this block if <expr> == false (therefore, not executing this block).
	jump_cond->arg = scope.last_inst;

	if (peek_token(c, 0).type == TOKEN_ELSE)
	{
		// these emit an additional jump at the beginning, in case this block ends up executing, so jump here instead.
		if (peek_token(c, 1).type == TOKEN_IF)
		{
			emit_else_if_statement(c);
			jump_cond->arg++;
		}
		else
		{
			emit_else_statement(c);
			jump_cond->arg++;
		}
	}
}

// else { ... }
static void emit_else_statement(PQ_Compiler* c) 
{
	PQ_Instruction* jump = push_inst(c, (PQ_Instruction){ INST_JUMP });

	// else
	eat_token(c);

	PQ_Scope scope = {};
	
	// {
	begin_scope(c, &scope);

	// Statements
	while (peek_token(c, 0).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(c);
	}

	// }
	end_scope(c, &scope);

	jump->arg = scope.last_inst;
}

// break
static void emit_break_statement(PQ_Compiler* c) 
{
	if (!c->current_loop)
	{
		C_ERROR("Cannot use break outside a loop");
	}

	// break
	eat_token(c);

	// argument is -1 so we know what instructions to patch in the loop statements.
	push_inst(c, (PQ_Instruction){ INST_JUMP, (uint16_t)-1 });
}

// foreign define <ident>(...)
static void emit_foreign_define_statement(PQ_Compiler* c)
{
	// foreign
	eat_token(c);

	// define
	PQ_Token define = try_eat_token(c, TOKEN_DEFINE);

	// <ident>
	PQ_Token ident = try_eat_token(c, TOKEN_IDENTIFIER);

	String name = str_copy_from_to(c->arena, c->source, ident.start, ident.end);

	if (variable_exists(c, name) || procedure_exists(c, name)) 
	{
		C_ERROR("Identifier '%.*s' redefined", s_fmt(name));
	}

	PQ_Procedure* proc = get_or_create_procedure(c, name);

	proc->foreign = true;

	// (
	try_eat_token(c, TOKEN_OPEN_PAREN);

	// <ident>, <ident>...
	while (peek_token(c, 0).type != TOKEN_CLOSE_PAREN)
	{
		// Check for (,<ident>
		if (peek_token(c, -1).type == TOKEN_OPEN_PAREN && peek_token(c, 0).type == TOKEN_COMMA)
		{
			C_ERROR("Expected expression after `(` in foreign procedure declaration, got `,`");
		}

		// <ident>
		try_eat_token(c, TOKEN_IDENTIFIER);

		proc->arg_count++;

		if (peek_token(c, 0).type != TOKEN_CLOSE_PAREN)
		{
			// ,
			try_eat_token(c, TOKEN_COMMA);
	
			// Check for <ident>,)
			if (peek_token(c, 0).type == TOKEN_CLOSE_PAREN)
			{
				C_ERROR("Expected expression after `,` in foreign procedure declaration, got `)`");
			}
		}
	}

	// )
	try_eat_token(c, TOKEN_CLOSE_PAREN);
}

static void emit_statement(PQ_Compiler* c)
{
	switch (peek_token(c, 0).type)
	{
		case TOKEN_VAR:
		{
			emit_var_statement(c);
		} break;

		case TOKEN_IDENTIFIER:
		{
			if (peek_token(c, 1).type == TOKEN_OPEN_PAREN)
			{
				emit_procedure_expression(c);
			}
			else if (peek_token(c, 1).type == TOKEN_OPEN_BOX)
			{
				emit_array_element_expression(c);
			}
			else if (pq_token_is_assign_op(peek_token(c, 1).type))			
			{
				emit_assign_expression(c);
			}
			else
			{
				emit_identifier_expression(c);
			}
		} break;

		case TOKEN_FOREIGN:
		{
			emit_foreign_define_statement(c);
		} break;

		case TOKEN_DEFINE:
		{
			emit_define_statement(c);
		} break;

		case TOKEN_RETURN:
		{
			emit_return_statement(c);
		} break;

		case TOKEN_REPEAT:
		{
			if (peek_token(c, 1).type == TOKEN_UNTIL)
			{
				emit_repeat_until_statement(c);
			}
			else
			{
				emit_repeat_statement(c);
			}
		} break;

		case TOKEN_FOREVER:
		{
			emit_forever_statement(c);
		} break;

		case TOKEN_BREAK:
		{
			emit_break_statement(c);
		} break;

		case TOKEN_IF:
		{
			emit_if_statement(c);
		} break;

		case TOKEN_OPEN_BRACE:
		{
			emit_scope(c);
		} break;

		// both of these statements cannot exist without an if statement, so these are
		// handled above. 
		case TOKEN_ELSE:
		{
			if (peek_token(c, 1).type == TOKEN_IF)
			{
				C_ERROR("Expected if statement before else if");
			}
			else
			{
				C_ERROR("Expected if or else if statement before else");
			}
		} break;

		default:
		{
			C_ERROR("Expected statement, got %s", pq_token_to_c_str(peek_token(c, 0).type));
		} break;
	}
}

static void generate(PQ_Compiler* c)
{
	c->idx = 0;

	while (c->idx < c->token_count)
	{
		emit_statement(c);
	}

	push_inst(c, (PQ_Instruction){ INST_HALT });
}

//
// interface
//

void pq_init_compiler(PQ_Compiler* c, Arena* arena, String source, PQ_CompilerErrorFn error)
{
	c->arena = arena;

	c->source = source;

	c->error = error;

	c->tokens = arena_push_array(c->arena, PQ_Token, PQ_MAX_TOKENS);
	c->token_count = 0;

	c->instructions = arena_push_array(c->arena, PQ_Instruction, PQ_MAX_INSTRUCTIONS);
	c->instruction_count = 0;

	c->immediates = arena_push_array(c->arena, PQ_Value, PQ_MAX_IMMEDIATES);
	c->immediate_count = 0;

	c->procedures = arena_push_array(c->arena, PQ_Procedure, PQ_MAX_PROCEDURES);
	c->procedure_count = 0;

	c->variables = arena_push_array(c->arena, PQ_Variable, PQ_MAX_VARIABLES);
	c->variable_count = 0;

	c->current_scope = nullptr;
	c->current_proc = nullptr;
	c->current_loop = nullptr;

	c->line = 1;
	c->idx = 0;
}

//
// serialization
//

static void write_to_blob(void* v, PQ_CompiledBlob* b, size_t type_size)
{
	__builtin_memcpy(b->buffer + b->size, v, type_size);
	b->size += type_size;
}

static void write_magic(PQ_Compiler* c, PQ_CompiledBlob* b)
{
	uint32_t magic = __builtin_bswap32('PIQR');
	write_to_blob(&magic, b, sizeof(uint32_t));
}

static void write_immediates(PQ_Compiler* c, PQ_CompiledBlob* b)
{
	write_to_blob(&c->immediate_count, b, sizeof(uint8_t));

	for (uint16_t i = 0; i < c->immediate_count; i++)
	{
		PQ_Value v = c->immediates[i];

		write_to_blob(&v.type, b, sizeof(PQ_ValueType));

		switch (v.type)
		{
			case VALUE_NULL: break;

			case VALUE_NUMBER:  write_to_blob(&v.n, b, sizeof(float)); break;
			case VALUE_BOOLEAN: write_to_blob(&v.b, b, sizeof(bool)); break;

			case VALUE_STRING:
			{
				for (size_t i = 0; i < v.s.length; i++)
				{
					write_to_blob(&v.s.buffer[i], b, sizeof(char));
				}

				char n = '\0';

				write_to_blob(&n, b, sizeof(char));
			} break;

			default: __builtin_unreachable(); break;
		}
	}
}

static void write_procedures(PQ_Compiler* c, PQ_CompiledBlob* b)
{
	write_to_blob(&c->procedure_count, b, sizeof(uint8_t));

	for (uint16_t i = 0; i < c->procedure_count; i++)
	{
		PQ_Procedure p = c->procedures[i];
		
		write_to_blob(&p.foreign, b, sizeof(bool));
		write_to_blob(&p.local_count, b, sizeof(uint8_t));
		write_to_blob(&p.arg_count, b, sizeof(uint8_t));
		write_to_blob(&p.scope.first_inst, b, sizeof(uint16_t));
	
		if (p.foreign)
		{
			for (size_t i = 0; i < p.name.length; i++)
			{
				write_to_blob(&p.name.buffer[i], b, sizeof(char));
			}

			char n = '\0';

			write_to_blob(&n, b, sizeof(char));
		}
	}
}

static void write_global_count(PQ_Compiler* c, PQ_CompiledBlob* b)
{
	uint16_t count = 0;

	for (uint16_t i = 0; i < c->variable_count; i++)
	{
		if (c->variables[i].global)
		{
			count++;
		}
	}

	write_to_blob(&count, b, sizeof(uint16_t));
}

static void write_instructions(PQ_Compiler* c, PQ_CompiledBlob* b)
{
	write_to_blob(&c->instruction_count, b, sizeof(uint16_t));

	for (uint16_t i = 0; i < c->instruction_count; i++)
	{
		PQ_Instruction it = c->instructions[i];

		write_to_blob(&it.type, b, sizeof(PQ_InstructionType));

		if (pq_inst_needs_arg(it.type))
		{
			write_to_blob(&it.arg, b, sizeof(uint16_t));
		}
	}
}

static void write_blob(PQ_Compiler* c, PQ_CompiledBlob* b)
{
	write_magic(c, b);
	write_immediates(c, b);
	write_procedures(c, b);
	write_global_count(c, b);
	write_instructions(c, b);
}

PQ_CompiledBlob pq_compile(PQ_Compiler* c)
{
	tokenize(c);
	generate(c);

	PQ_CompiledBlob b = {};

	b.buffer = arena_push_array(c->arena, uint8_t, PQ_MAX_BLOB_SIZE);
	b.size = 0;	

	write_blob(c, &b);

	return b;
}

void pq_declare_foreign_proc(PQ_Compiler* c, String name, uint8_t arg_count)
{
	if (procedure_exists(c, name))
	{
		C_ERROR("Procedure '%.*s' redeclared", s_fmt(name));
	}

	PQ_Procedure* proc = get_or_create_procedure(c, name);

	proc->foreign = true;
	proc->arg_count = arg_count;
}

#undef C_ERROR