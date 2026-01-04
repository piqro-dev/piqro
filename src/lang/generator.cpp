
#include <lang/generator.h>

static inline bool identifier_too_long(Token t)
{
	ASSERT(t.type == TOKEN_IDENTIFIER);

	return t.end - t.start - 1 > MAX_IDENTIFIER_NAME_LENGTH;
}

static inline Token peek_token(const Generator* gen, int16_t offset = 0)
{
	if (gen->ptr + offset > gen->tokens.count)
	{
		return Token{ TOKEN_UNKNOWN, peek_token(gen, -1).line };
	}

	return gen->tokens[gen->ptr + offset];
}

static inline Token eat_token(Generator* gen)
{
	Token t = gen->tokens[gen->ptr++];

	gen->line = t.line;

	return t;
}

__attribute__((format(printf, 2, 3)))
static inline void error(const Generator* gen, const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	errorln("Error: line %d: %s", gen->line, out);
}

static inline Token try_eat_token(Generator* gen, TokenType expected)
{
	const Token peek = peek_token(gen);

	if (peek.type != expected)
	{
		error(gen, "Expected %s, got %s", to_string(expected), to_string(peek.type));
	}

	return gen->tokens[gen->ptr++];
}

static inline bool variable_exists(const Generator* gen, const char* name)
{
	for (uint16_t i = 0; i < gen->variables.count; i++)
	{
		if (__builtin_strcmp(gen->variables[i], name) == 0)
		{
			return true;
		}
	}

	return false;
}

static inline bool procedure_exists(const Generator* gen, const char* name)
{
	for (uint16_t i = 0; i < gen->procedures.count; i++)
	{
		if (__builtin_strcmp(gen->procedures[i].name, name) == 0)
		{
			return true;
		}
	}

	return false;
}

static inline Procedure* get_or_create_procedure(Generator* gen, const char* name)
{
	for (uint16_t i = 0; i < gen->procedures.count; i++)
	{
		if (__builtin_strcmp(gen->procedures[i].name, name) == 0)
		{
			return &gen->procedures[i];
		}
	}

	Procedure* proc = push(&gen->procedures);
	__builtin_strcpy(proc->name, name);

	return proc;
}

static inline uint16_t get_or_create_variable(Generator* gen, const char* name)
{
	for (uint16_t i = 0; i < gen->variables.count; i++)
	{
		if (__builtin_strcmp(gen->variables[i], name) == 0)
		{
			return i;
		}
	}

	__builtin_strcpy(*push(&gen->variables), name);

	return gen->variables.count - 1;
}

static inline uint16_t get_or_create_immediate(Generator* gen, Value v)
{
	for (uint16_t i = 0; i < gen->immediates.count; i++)
	{
		if (as_number(&gen->immediates[i]) == as_number(&v))
		{
			return i;
		}
	}

	push(&gen->immediates, v);

	return gen->immediates.count - 1;
}

static inline void emit_expression(Generator* gen, Array <Instruction>* out);

static inline void emit_statement(Generator* gen, Array <Instruction>* out);

//
// Expressions
//

static inline void emit_number_expression(Generator* gen, Array <Instruction>* out)
{
	Token number = eat_token(gen);

	// A number might be longer than this, not sure if it needs attention?
	char buf[256] = {};
	as_string(number, gen->source, buf, 256);

	Value imm = make_value((float)atof(buf));

	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, imm));
}

static inline void emit_identifier_expression(Generator* gen, Array <Instruction>* out)
{
	Token ident = eat_token(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!variable_exists(gen, name))
	{
		error(gen, "Undefined identifier '%s'", name);
	}

	emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));
}

static inline void emit_boolean_expression(Generator* gen, Array <Instruction>* out)
{
	Token boolean = eat_token(gen);

	if (boolean.type == TOKEN_TRUE)
	{
		emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(true)));
	}
	else if (boolean.type == TOKEN_FALSE)
	{
		emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(false)));
	}
}

// <ident>(<expr>, <expr>...)
static inline void emit_procedure_expression(Generator* gen, Array <Instruction>* out)
{
	Token ident = eat_token(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	// May not exist
	if (!procedure_exists(gen, name))
	{
		error(gen, "Undefined procedure '%s'", name);
	}

	// (
	Token open = eat_token(gen);

	uint8_t arg_count = 0; 

	// <expr>, <expr>...
	while (peek_token(gen).type != TOKEN_CLOSE_BRACE)
	{
		// Check for (,<expr>
		if (peek_token(gen, -1).type == TOKEN_OPEN_BRACE && peek_token(gen).type == TOKEN_COMMA)
		{
			error(gen, "Expected expression after `(` in procedure call, got `,`");
		}

		// <expr>
		emit_expression(gen, out);

		arg_count++;

		if (peek_token(gen).type != TOKEN_CLOSE_BRACE)
		{
			// ,
			try_eat_token(gen, TOKEN_COMMA);
	
			// Check for <expr>,)
			if (peek_token(gen).type == TOKEN_CLOSE_BRACE)
			{
				error(gen, "Expected argument after `,` in procedure call, got `)`");
			}
		}
	}

	Procedure* proc = get_or_create_procedure(gen, name);

	if (proc->arg_count != arg_count)
	{
		error(gen, "Expected %d arguments for procedure '%s', got %d", proc->arg_count, name, arg_count);
	}

	// )
	Token close = try_eat_token(gen, TOKEN_CLOSE_BRACE);

	// Push the argument count to be the last
	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value((float)proc->arg_count)));

	// Call the procedure
	emplace(out, INSTRUCTION_CALL, proc->scope->first_inst);
}

static inline void emit_expression(Generator* gen, Array <Instruction>* out)
{
	Token expr = peek_token(gen);

	if (expr.type == TOKEN_NUMBER)
	{
		emit_number_expression(gen, out);
	}
	else if (expr.type == TOKEN_IDENTIFIER)
	{
		if (peek_token(gen, 1).type == TOKEN_OPEN_BRACE)
		{
			emit_procedure_expression(gen, out);
		}
		else
		{
			emit_identifier_expression(gen, out);
		}
	}
	else if (expr.type == TOKEN_TRUE || expr.type == TOKEN_FALSE)
	{
		emit_boolean_expression(gen, out);
	}
	else
	{
		error(gen, "Expected expression");
	}
}

//
// Scope
//

static inline Scope* begin_scope(Generator* gen, Array <Instruction>* out) 
{
	// {
	Token open = eat_token(gen);

	Scope* scope = push(&gen->scopes);

	scope->first_inst = out->count + 1;
	scope->local_base = gen->variables.count;

	return scope;
}

static inline void end_scope(Generator* gen, Array <Instruction>* out) 
{
	// }
	Token close = try_eat_token(gen, TOKEN_CLOSE_CURLY);

	Scope* scope = pop(&gen->scopes);

	scope->last_inst = out->count;

	trim_end(&gen->variables, scope->local_base);
}

static inline void emit_scope(Generator* gen, Array <Instruction>* out)
{
	begin_scope(gen, out);

	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	end_scope(gen, out);
}

//
// Statements
//

// <ident> = <expr>
static inline void emit_assign_statement(Generator* gen, Array <Instruction>* out)
{
	// <ident> 
	Token ident = eat_token(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!variable_exists(gen, name)) 
	{
		error(gen, "Undefined identifier '%s'", name);
	}

	// =
	Token eq = try_eat_token(gen, TOKEN_EQUALS);

	// <expr>
	emit_expression(gen, out);

	emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
}

// var <ident> = <expr>
static inline void emit_var_statement(Generator* gen, Array <Instruction>* out)
{
	// var
	Token var = eat_token(gen);
	
	// <ident>
	Token ident = try_eat_token(gen, TOKEN_IDENTIFIER);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (identifier_too_long(ident))
	{
		error(gen, "Identifier '%s' is too long", name);
	}

	if (variable_exists(gen, name) || procedure_exists(gen, name)) 
	{
		error(gen, "Identifier '%s' redefined", name);
	}

	// =
	try_eat_token(gen, TOKEN_EQUALS);

	// <expr>
	emit_expression(gen, out);

	emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
}

// define <ident>(<ident>, <ident>...) { ... }
static inline void emit_define_statement(Generator* gen, Array <Instruction>* out)
{
	// define
	Token define = eat_token(gen);

	if (gen->current_procedure)
	{
		error(gen, "Cannot define procedure inside another one");
	}

	// <ident>
	Token ident = try_eat_token(gen, TOKEN_IDENTIFIER);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (variable_exists(gen, name) || procedure_exists(gen, name)) 
	{
		error(gen, "Identifier '%s' redefined", name);
	}

	Procedure* proc = get_or_create_procedure(gen, name);

	// (
	Token open = try_eat_token(gen, TOKEN_OPEN_BRACE);

	// <ident>, <ident>...
	while (peek_token(gen).type != TOKEN_CLOSE_BRACE)
	{
		// Check for (,<ident>
		if (peek_token(gen, -1).type == TOKEN_OPEN_BRACE && peek_token(gen).type == TOKEN_COMMA)
		{
			error(gen, "Expected argument after `(` in procedure definition, got `,`");
		}

		// <ident>
		Token ident = try_eat_token(gen, TOKEN_IDENTIFIER);

		Identifier name = {};
		as_string(ident, gen->source, name, 256);

		if (variable_exists(gen, name))
		{
			error(gen, "Argument/variable redefinition in procedure definition");
		}

		get_or_create_variable(gen, name);

		proc->arg_count++;

		if (peek_token(gen).type != TOKEN_CLOSE_BRACE)
		{
			// ,
			try_eat_token(gen, TOKEN_COMMA);
		
			// Check for <ident>,)
			if (peek_token(gen).type == TOKEN_CLOSE_BRACE)
			{
				error(gen, "Expected argument after `,` in procedure definition, got `)`");
			}
		}
	}

	// )
	Token close = try_eat_token(gen, TOKEN_CLOSE_BRACE);

	proc->scope = begin_scope(gen, out);

	// Move the local index back to the first argument
	proc->scope->local_base = gen->variables.count - proc->arg_count;

	gen->current_procedure = proc;

	// Save it, as we only know the last instruction after the
	// end_scope(); 
	Instruction* jump = emplace(out, INSTRUCTION_JUMP);

	// { ... }
	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	// If we didn't emit a `return`, return nothing. 
	if (!proc->returns_value)
	{
		emplace(out, INSTRUCTION_LOAD_NULL);
		emplace(out, INSTRUCTION_RET);
	}

	end_scope(gen, out);

	// We know the last instruction now, jump here.
	jump->arg = proc->scope->last_inst;

	gen->current_procedure = nullptr;
}

// return <expr>
static inline void emit_return_statement(Generator* gen, Array <Instruction>* out)
{
	if (!gen->current_procedure)
	{
		error(gen, "Cannot use return outside procedure");
	}

	// At some point in the procedure we will return a value that's not UNDEFINED. 
	// TODO: Might be an issue if you return in an if statement
	gen->current_procedure->returns_value = true;

	// return
	Token return_ = eat_token(gen);

	// <expr>
	emit_expression(gen, out);

	emplace(out, INSTRUCTION_RET);
}

static inline void emit_statement(Generator* gen, Array <Instruction>* out)
{
	// var <ident> = <expr>
	if (peek_token(gen).type == TOKEN_VAR)
	{
		emit_var_statement(gen, out);	
	}
	else if (peek_token(gen).type == TOKEN_IDENTIFIER)
	{
		// <ident>(...)
		if (peek_token(gen, 1).type == TOKEN_OPEN_BRACE)
		{
			emit_procedure_expression(gen, out);
		}
		// <ident> = <expr>
		else
		{
			emit_assign_statement(gen, out);
		}
	}
	// define <ident>(...) { ... }
	else if (peek_token(gen).type == TOKEN_DEFINE)
	{
		emit_define_statement(gen, out);
	}
	// return <expr>
	else if (peek_token(gen).type == TOKEN_RETURN)
	{
		emit_return_statement(gen, out);
	}
	// { ... }
	else if (peek_token(gen).type == TOKEN_OPEN_CURLY)
	{
		emit_scope(gen, out);
	}
	else
	{
		error(gen, "Expected statement, got %s", to_string(peek_token(gen).type));
	}
}

//
// Public code
//

static inline void init(Generator* gen, Arena* arena, const char* source, Array <Token> tokens)
{
	gen->source = source;

	gen->tokens = tokens;

	gen->immediates = make_array<Value>(arena, MAX_IMMEDIATES);
	gen->variables = make_array<Identifier>(arena, MAX_VARIABLES);
	gen->scopes = make_array<Scope>(arena, MAX_SCOPES);
	gen->procedures = make_array<Procedure>(arena, MAX_PROCEDURES);

	gen->current_procedure = nullptr;
	
	gen->line = 1;
	gen->ptr = 0;
}

static inline void emit_program(Generator* gen, Array <Instruction>* out)
{
	while (gen->ptr < gen->tokens.count)
	{
		emit_statement(gen, out);
	}

	// Ensure we terminate the program at the end
	emplace(out, INSTRUCTION_HALT);
}