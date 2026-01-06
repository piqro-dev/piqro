
#include <lang/generator.h>

inline bool identifier_too_long(Token t)
{
	ASSERT(t.type == TOKEN_IDENTIFIER);

	return t.end - t.start - 1 > MAX_IDENTIFIER_NAME_LENGTH;
}

inline Token peek_token(const Generator* gen, int16_t offset = 0)
{
	if (gen->ptr + offset > gen->tokens.count)
	{
		return Token{ TOKEN_UNKNOWN, peek_token(gen, -1).line };
	}

	return gen->tokens[gen->ptr + offset];
}

inline Token eat_token(Generator* gen)
{
	Token t = gen->tokens[gen->ptr++];

	gen->line = t.line;

	return t;
}

__attribute__((format(printf, 2, 3)))
inline void error(const Generator* gen, const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	errorln("Error: line %d: %s", gen->line, out);
}

inline Token try_eat_token(Generator* gen, TokenType expected)
{
	const Token peek = peek_token(gen);

	if (peek.type != expected)
	{
		error(gen, "Expected %s, got %s", to_string(expected), to_string(peek.type));
	}

	return gen->tokens[gen->ptr++];
}

inline bool variable_exists(const Generator* gen, const char* name)
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

inline bool procedure_exists(const Generator* gen, const char* name)
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

inline Procedure* get_or_create_procedure(Generator* gen, const char* name)
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

inline uint16_t get_or_create_variable(Generator* gen, const char* name)
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

inline uint16_t get_or_create_immediate(Generator* gen, Value v)
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

inline void emit_expression(Generator* gen, Array <Instruction>* out);

inline void emit_statement(Generator* gen, Array <Instruction>* out);

//
// Expressions
//

inline void emit_number_expression(Generator* gen, Array <Instruction>* out)
{
	Token number = eat_token(gen);

	// A number might be longer than this, not sure if it needs attention?
	char buf[256] = {};
	as_string(number, gen->source, buf, 256);

	Value imm = make_value((float)atof(buf));

	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, imm));
}

inline void emit_identifier_expression(Generator* gen, Array <Instruction>* out)
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

inline void emit_boolean_expression(Generator* gen, Array <Instruction>* out)
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
inline void emit_procedure_expression(Generator* gen, Array <Instruction>* out)
{
	Token ident = eat_token(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!procedure_exists(gen, name))
	{
		error(gen, "Undefined procedure '%s'", name);
	}

	// (
	eat_token(gen);

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
				error(gen, "Expected expression after `,` in procedure call, got `)`");
			}
		}
	}

	Procedure* proc = get_or_create_procedure(gen, name);

	if (proc->arg_count != arg_count)
	{
		error(gen, "Expected %d arguments for procedure '%s', got %d", proc->arg_count, name, arg_count);
	}

	// )
	try_eat_token(gen, TOKEN_CLOSE_BRACE);

	// Push the argument count to be the last
	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value((float)proc->arg_count)));

	// Call the procedure
	emplace(out, INSTRUCTION_CALL, proc->scope.first_inst);
}

// <ident> =... <expr>
inline void emit_assign_expression(Generator* gen, Array <Instruction>* out) 
{
	// <ident> 
	Token ident = eat_token(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!variable_exists(gen, name)) 
	{
		error(gen, "Undefined identifier '%s'", name);
	}

	// =...
	Token assign = eat_token(gen);

	switch (assign.type)
	{
		case TOKEN_EQUALS: break;

		case TOKEN_PLUS_EQUALS: 
		{
			emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));

			// <expr>
			emit_expression(gen, out);

			// +=
			emplace(out, INSTRUCTION_ADD);
			emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
		} break;

		case TOKEN_DASH_EQUALS: 
		{
			emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));

			// <expr>
			emit_expression(gen, out);

			// -=
			emplace(out, INSTRUCTION_SUB);
			emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
		} break;

		case TOKEN_SLASH_EQUALS: 
		{
			emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));

			// <expr>
			emit_expression(gen, out);

			// /=
			emplace(out, INSTRUCTION_DIV);
			emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
		} break;

		case TOKEN_STAR_EQUALS: 
		{
			emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));

			// <expr>
			emit_expression(gen, out);

			// *=
			emplace(out, INSTRUCTION_MUL);
			emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
		} break;

		case TOKEN_PERCENT_EQUALS: 
		{
			emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));

			// <expr>
			emit_expression(gen, out);

			// %=
			emplace(out, INSTRUCTION_MOD);
			emplace(out, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
		} break;


		default: 
		{
			error(gen, "Unexpected %s", to_string(assign.type));
		}
	}

	emplace(out, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));
}

// <expr> <op> <expr>
// Based on https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
inline void emit_binary_expression(Generator* gen, Array <Instruction>* out, int8_t min_precedence = -1)
{
	// Avoid recursion
	if (min_precedence > -1) 
	{
		// Left <expr>
		emit_expression(gen, out);
	}

	// <op>
	Token op = eat_token(gen);

	// Right <expr>
	emit_expression(gen, out);

	switch (op.type)
	{
		case TOKEN_PERCENT: emplace(out, INSTRUCTION_MOD); break; 
		case TOKEN_PLUS:    emplace(out, INSTRUCTION_ADD); break; 
		case TOKEN_DASH:    emplace(out, INSTRUCTION_SUB); break;
		case TOKEN_SLASH:   emplace(out, INSTRUCTION_DIV); break; 
		case TOKEN_STAR:    emplace(out, INSTRUCTION_MUL); break; 

		case TOKEN_LESS:         emplace(out, INSTRUCTION_LESS); break; 
		case TOKEN_GREATER:      emplace(out, INSTRUCTION_GREATER); break; 
		case TOKEN_LESS_THAN:    emplace(out, INSTRUCTION_LESS_THAN); break; 
		case TOKEN_GREATER_THAN: emplace(out, INSTRUCTION_GREATER_THAN); break; 
		
		case TOKEN_DOUBLE_EQUALS: emplace(out, INSTRUCTION_EQUALS); break; 
		case TOKEN_NOT_EQUALS:    emplace(out, INSTRUCTION_EQUALS); emplace(out, INSTRUCTION_NOT); break; 
		
		case TOKEN_DOUBLE_AND:  emplace(out, INSTRUCTION_AND); break; 
		case TOKEN_DOUBLE_PIPE: emplace(out, INSTRUCTION_OR); break; 
		
		default: { error(gen, "TODO: Not implemented: %s", to_string(op.type)); }
	}

	// Next token
	op = peek_token(gen);

	while (is_binary_op(op.type) && precedence_of(op.type) >= min_precedence)
	{
		min_precedence = precedence_of(op.type) + 1;
		emit_binary_expression(gen, out, min_precedence);
	}
}

inline void emit_expression(Generator* gen, Array <Instruction>* out)
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
		else if (peek_token(gen, 1).type == TOKEN_EQUALS)
		{
			emit_assign_expression(gen, out);
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
		error(gen, "Expected expression, got %s", to_string(expr.type));
	}

	if (is_binary_op(peek_token(gen).type))
	{
		emit_binary_expression(gen, out);
	}
}

//
// Scope
//

inline void begin_scope(Generator* gen, Array <Instruction>* out, Scope* scope) 
{
	// {
	try_eat_token(gen, TOKEN_OPEN_CURLY);

	scope->first_inst = out->count;
	scope->local_base = gen->variables.count;

	gen->current_scope = scope;
}

inline void end_scope(Generator* gen, Array <Instruction>* out, Scope* scope) 
{
	// }
	try_eat_token(gen, TOKEN_CLOSE_CURLY);

	scope->last_inst = out->count;

	trim_end(&gen->variables, scope->local_base);

	gen->current_scope = nullptr;
}

inline void emit_scope(Generator* gen, Array <Instruction>* out)
{
	Scope scope = {};

	begin_scope(gen, out, &scope);

	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	end_scope(gen, out, &scope);
}

//
// Statements
//

// var <ident> = <expr>
inline void emit_var_statement(Generator* gen, Array <Instruction>* out)
{
	// var
	eat_token(gen);
	
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
inline void emit_define_statement(Generator* gen, Array <Instruction>* out)
{
	// define
	eat_token(gen);

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
	try_eat_token(gen, TOKEN_CLOSE_BRACE);

	// Definitions are skipped initiallly
	Instruction* jump = emplace(out, INSTRUCTION_JUMP);

	// {
	begin_scope(gen, out, &proc->scope);

	// Move the local index back to the first argument
	proc->scope.local_base = gen->variables.count - proc->arg_count;

	gen->current_procedure = proc;

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

	// }
	end_scope(gen, out, &proc->scope);

	gen->current_procedure = nullptr;

	// We know the last instruction now, we'll jump here.
	jump->arg = proc->scope.last_inst;
}

// return <expr>
inline void emit_return_statement(Generator* gen, Array <Instruction>* out)
{
	if (!gen->current_procedure)
	{
		error(gen, "Cannot use return outside procedure");
	}

	// At some point in the procedure we will return a value that's not UNDEFINED. 
	// NOTE: Might be an issue if a value is not returned every path
	gen->current_procedure->returns_value = true;

	// return
	eat_token(gen);

	// <expr>
	emit_expression(gen, out);

	emplace(out, INSTRUCTION_RET);
}

inline void patch_breaks(const Loop* loop, Array <Instruction>* instructions) 
{
	for (uint16_t i = loop->scope.first_inst; i < loop->scope.last_inst; i++) 
	{
		Instruction* it = &(*instructions)[i];

		if (it->type == INSTRUCTION_JUMP && it->arg == (uint16_t)-1) 
		{
			it->arg = loop->scope.last_inst + 1;
		}
	}
}

//
// The statement `repeat <expr> { ... }` replicates the equivalent of this C code:
//
// int repeat_local = <expr>;
//
// while (--repeat_local >= 0) 
// {
//   <...>
// }
// 
inline void emit_repeat_statement(Generator* gen, Array <Instruction>* out)
{
	// repeat
	eat_token(gen);

	// <expr>
	emit_expression(gen, out);

	Loop loop = {};
	
	// {
	begin_scope(gen, out, &loop.scope);

	Identifier name = {};
	__builtin_sprintf(name, "repeat_local_%d", loop.scope.local_base);

	loop.local = get_or_create_variable(gen, name);

	// repeat_local = <expr>
	emplace(out, INSTRUCTION_STORE_LOCAL, loop.local);

	// repeat_local >= 0
	emplace(out, INSTRUCTION_LOAD_LOCAL, loop.local);
	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(0.0f)));
	emplace(out, INSTRUCTION_LESS_THAN);

	// True? Skip the loop
	Instruction* jump_cond = emplace(out, INSTRUCTION_JUMP_COND);

	// repeat_local = repeat_local - 1
	emplace(out, INSTRUCTION_LOAD_LOCAL, loop.local);
	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(1.0f)));
	emplace(out, INSTRUCTION_SUB);
	emplace(out, INSTRUCTION_STORE_LOCAL, loop.local);

	gen->current_loop = &loop;

	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	gen->current_loop = nullptr;

	end_scope(gen, out, &loop.scope);

	emplace(out, INSTRUCTION_JUMP, loop.scope.first_inst);

	jump_cond->arg = loop.scope.last_inst + 1; // Last statement + the jump

	// Patch breaks
	patch_breaks(&loop, out);
}

// forever { ... }
inline void emit_forever_statement(Generator* gen, Array <Instruction>* out)
{
	// forever
	eat_token(gen);	

	Loop loop = {};

	// {
	begin_scope(gen, out, &loop.scope);
	
	gen->current_loop = &loop;

	// Statements
	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	gen->current_loop = nullptr;

	// }
	end_scope(gen, out, &loop.scope);

	emplace(out, INSTRUCTION_JUMP, loop.scope.first_inst);

	patch_breaks(&loop, out);
}

inline void emit_else_if_statement(Generator* gen, Array <Instruction>* out);

inline void emit_else_statement(Generator* gen, Array <Instruction>* out);

// if <expr> { ... }
inline void emit_if_statement(Generator* gen, Array <Instruction>* out)
{
	// if
	eat_token(gen);

	// <expr>
	emit_expression(gen, out);

	// <expr> == false?
	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(false)));
	emplace(out, INSTRUCTION_EQUALS);

	// True? Skip the block.
	Instruction* jump_cond = emplace(out, INSTRUCTION_JUMP_COND);

	Scope scope = {};

	// {
	begin_scope(gen, out, &scope);

	// Statements
	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}
 
	// }
	end_scope(gen, out, &scope);

	// This jumps to the end of this block if <expr> == false (therefore, not executing this block).
	jump_cond->arg = scope.last_inst;

	if (peek_token(gen).type == TOKEN_ELSE)
	{
		// These emit an additional jump at the beginning, in case this block ends up executing, so jump here instead.
		if (peek_token(gen, 1).type == TOKEN_IF)
		{
			emit_else_if_statement(gen, out);
			jump_cond->arg++;
		}
		else
		{
			emit_else_statement(gen, out);
			jump_cond->arg++;
		}
	}
}

// else if <expr> { ... }
inline void emit_else_if_statement(Generator* gen, Array <Instruction>* out) 
{
	// If the last block executes, this will skip this block.
	Instruction* jump = emplace(out, INSTRUCTION_JUMP);

	// else if
	eat_token(gen);
	eat_token(gen);

	// <expr>
	emit_expression(gen, out);

	// <expr> == false?
	emplace(out, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(false)));
	emplace(out, INSTRUCTION_EQUALS);

	// True? Skip the block.
	Instruction* jump_cond = emplace(out, INSTRUCTION_JUMP_COND);

	Scope scope = {};

	// {
	begin_scope(gen, out, &scope);

	// Statements
	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	// }
	end_scope(gen, out, &scope);

	// This jumps to the end of this block if the last block was executed. 
	jump->arg = scope.last_inst;

	// This jumps to the end of this block if <expr> == false (therefore, not executing this block).
	jump_cond->arg = scope.last_inst;

	if (peek_token(gen).type == TOKEN_ELSE)
	{
		// These emit an additional jump at the beginning, in case this block ends up executing, so jump here instead.
		if (peek_token(gen, 1).type == TOKEN_IF)
		{
			emit_else_if_statement(gen, out);
			jump_cond->arg++;
		}
		else
		{
			emit_else_statement(gen, out);
			jump_cond->arg++;
		}
	}
}

// else { ... }
inline void emit_else_statement(Generator* gen, Array <Instruction>* out) 
{
	Instruction* jump = emplace(out, INSTRUCTION_JUMP);

	// else
	eat_token(gen);

	Scope scope = {};
	
	// {
	begin_scope(gen, out, &scope);

	// Statements
	while (peek_token(gen).type != TOKEN_CLOSE_CURLY)
	{
		emit_statement(gen, out);
	}

	// }
	end_scope(gen, out, &scope);

	jump->arg = scope.last_inst;
}

// break
inline void emit_break_statement(Generator* gen, Array <Instruction>* out) 
{
	if (!gen->current_loop)
	{
		error(gen, "Cannot use break outside a loop.");
	}

	// break
	eat_token(gen);

	// Argument is -1 so we know what instructions to patch in the loop statements.
	emplace(out, INSTRUCTION_JUMP, (uint16_t)-1);
}

inline void emit_statement(Generator* gen, Array <Instruction>* out)
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
			emit_assign_expression(gen, out);
			pop(out); // Pop the LOAD_IMMEDIATE if this is a statement.
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
	// repeat <expr> { ... }
	else if (peek_token(gen).type == TOKEN_REPEAT)
	{
		emit_repeat_statement(gen, out);
	}
	// forever { ... }
	else if (peek_token(gen).type == TOKEN_FOREVER)
	{
		emit_forever_statement(gen, out);
	}
	// if <expr> { ... } else if <expr> { ... } else { ... }
	else if (peek_token(gen).type == TOKEN_IF)
	{
		emit_if_statement(gen, out);
	}
	// break
	else if (peek_token(gen).type == TOKEN_BREAK)
	{
		emit_break_statement(gen, out);
	}
	// Both of these statements cannot exist without an if statement, so these are
	// handled above. 
	else if (peek_token(gen).type == TOKEN_ELSE)
	{
		if (peek_token(gen, 1).type == TOKEN_IF)
		{
			error(gen, "Expected if statement before else if.");
		}
		else
		{
			error(gen, "Expected if or else if statement before else.");
		}
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

void init(Generator* gen, Arena* arena, const char* source, Array <Token> tokens)
{
	gen->source = source;

	gen->tokens = tokens;

	gen->immediates = make_array<Value>(arena, MAX_IMMEDIATES);
	gen->variables = make_array<Identifier>(arena, MAX_VARIABLES);
	gen->procedures = make_array<Procedure>(arena, MAX_PROCEDURES);

	gen->current_procedure = nullptr;
	gen->current_loop = nullptr;
	gen->current_scope = nullptr;
	
	gen->line = 1;
	gen->ptr = 0;
}

void emit_program(Generator* gen, Array <Instruction>* out)
{
	while (gen->ptr < gen->tokens.count)
	{
		emit_statement(gen, out);
	}

	// Ensure we terminate the program at the end
	emplace(out, INSTRUCTION_HALT);
}