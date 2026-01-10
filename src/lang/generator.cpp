
#include <lang/generator.h>

inline bool identifier_too_long(Token t)
{
	ASSERT(t.type == TOKEN_IDENTIFIER);

	return t.end - t.start - 1 > MAX_IDENTIFIER_NAME_LENGTH;
}

inline Token peek(const Generator* gen, int16_t offset = 0)
{
	if (gen->ptr + offset > gen->tokens.count)
	{
		return Token{ TOKEN_UNKNOWN, gen->line };
	}

	return gen->tokens[gen->ptr + offset];
}

inline Token eat(Generator* gen)
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

inline Token try_eat(Generator* gen, TokenType expected)
{
	const Token next = peek(gen);

	if (next.type != expected)
	{
		error(gen, "Expected %s, got %s", to_string(expected), to_string(next.type));
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
	proc->idx = gen->procedures.count - 1;

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
		if (as_number(gen->immediates[i]) == as_number(v))
		{
			return i;
		}
	}

	push(&gen->immediates, v);

	return gen->immediates.count - 1;
}

inline void emit_expression(Generator* gen);

inline void emit_statement(Generator* gen);

//
// expressions
//

inline void emit_number_expression(Generator* gen)
{
	Token number = eat(gen);

	// a number might be longer than this, not sure if it needs attention?
	char buf[256] = {};
	as_string(number, gen->source, buf, 256);

	emplace(gen->instructions, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value((float)atof(buf))));
}

inline void emit_identifier_expression(Generator* gen)
{
	Token ident = eat(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!variable_exists(gen, name))
	{
		error(gen, "Undefined identifier '%s'", name);
	}

	emplace(gen->instructions, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));
}

inline void emit_boolean_expression(Generator* gen)
{
	Token boolean = eat(gen);

	bool r = false;

	switch (boolean.type)
	{
		case TOKEN_TRUE:  r = true; break;
		case TOKEN_FALSE: r = false; break;

		default: break;
	}

	emplace(gen->instructions, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(r)));
}

// <ident>(<expr>, <expr>...)
inline void emit_procedure_expression(Generator* gen)
{
	Token ident = eat(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!procedure_exists(gen, name))
	{
		error(gen, "Undefined procedure '%s'", name);
	}

	// (
	eat(gen);

	uint8_t arg_count = 0; 

	// <expr>, <expr>...
	while (peek(gen).type != TOKEN_CLOSE_PAREN)
	{
		// Check for (,<expr>
		if (peek(gen, -1).type == TOKEN_OPEN_PAREN && peek(gen).type == TOKEN_COMMA)
		{
			error(gen, "Expected expression after `(` in procedure call, got `,`");
		}

		// <expr>
		emit_expression(gen);

		arg_count++;

		if (peek(gen).type != TOKEN_CLOSE_PAREN)
		{
			// ,
			try_eat(gen, TOKEN_COMMA);
	
			// Check for <expr>,)
			if (peek(gen).type == TOKEN_CLOSE_PAREN)
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
	try_eat(gen, TOKEN_CLOSE_PAREN);

	// Call the procedure
	emplace(gen->instructions, INSTRUCTION_CALL, proc->idx);
}

// <ident> =... <expr>
inline void emit_assign_expression(Generator* gen) 
{
	// <ident> 
	Token ident = eat(gen);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (!variable_exists(gen, name)) 
	{
		error(gen, "Undefined identifier '%s'", name);
	}

	// =...
	Token assign = eat(gen);

	if (assign.type != TOKEN_EQUALS)
	{
		emplace(gen->instructions, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));
	}
	
	emit_expression(gen);

	switch (assign.type)
	{
		case TOKEN_EQUALS: break;

		case TOKEN_PLUS_EQUALS:    emplace(gen->instructions, INSTRUCTION_ADD); break;
		case TOKEN_DASH_EQUALS:    emplace(gen->instructions, INSTRUCTION_SUB); break;
		case TOKEN_SLASH_EQUALS:   emplace(gen->instructions, INSTRUCTION_DIV); break;
		case TOKEN_STAR_EQUALS:    emplace(gen->instructions, INSTRUCTION_MUL); break;
		case TOKEN_PERCENT_EQUALS: emplace(gen->instructions, INSTRUCTION_MOD); break;

		default: error(gen, "Unexpected %s", to_string(assign.type)); break;
	}
	
	emplace(gen->instructions, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
	emplace(gen->instructions, INSTRUCTION_LOAD_LOCAL, get_or_create_variable(gen, name));
}

// <expr> <op> <expr>
// based on https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
inline void emit_binary_expression(Generator* gen, int8_t min_precedence = -1)
{
	// avoid recursion
	if (min_precedence > -1) 
	{
		// left <expr>
		emit_expression(gen);
	}

	// <op>
	Token op = eat(gen);

	// right <expr>
	emit_expression(gen);

	switch (op.type)
	{
		case TOKEN_PERCENT:       emplace(gen->instructions, INSTRUCTION_MOD); break; 
		case TOKEN_PLUS:          emplace(gen->instructions, INSTRUCTION_ADD); break; 
		case TOKEN_DASH:          emplace(gen->instructions, INSTRUCTION_SUB); break;
		case TOKEN_SLASH:         emplace(gen->instructions, INSTRUCTION_DIV); break; 
		case TOKEN_STAR:          emplace(gen->instructions, INSTRUCTION_MUL); break; 

		case TOKEN_LESS:          emplace(gen->instructions, INSTRUCTION_LESS); break; 
		case TOKEN_GREATER:       emplace(gen->instructions, INSTRUCTION_GREATER); break; 
		case TOKEN_LESS_THAN:     emplace(gen->instructions, INSTRUCTION_LESS_THAN); break; 
		case TOKEN_GREATER_THAN:  emplace(gen->instructions, INSTRUCTION_GREATER_THAN); break; 
		
		case TOKEN_DOUBLE_EQUALS: emplace(gen->instructions, INSTRUCTION_EQUALS); break; 
		case TOKEN_NOT_EQUALS:    emplace(gen->instructions, INSTRUCTION_EQUALS); emplace(gen->instructions, INSTRUCTION_NOT); break; 
		
		case TOKEN_DOUBLE_AND:    emplace(gen->instructions, INSTRUCTION_AND); break; 
		case TOKEN_DOUBLE_PIPE:   emplace(gen->instructions, INSTRUCTION_OR); break; 
		
		default: { error(gen, "Expected binary operator, got %s", to_string(op.type)); }
	}

	// next token
	op = peek(gen);

	while (is_binary_op(op.type) && precedence_of(op.type) >= min_precedence)
	{
		min_precedence = precedence_of(op.type) + 1;
		emit_binary_expression(gen, min_precedence);
	}
}

inline void emit_expression(Generator* gen)
{
	Token expr = peek(gen);

	switch (expr.type)
	{
		case TOKEN_DASH:
		{
			eat(gen);
			emit_expression(gen);
			emplace(gen->instructions, INSTRUCTION_NEGATE);
		} break;

		case TOKEN_OPEN_PAREN:
		{
			eat(gen);
			emit_expression(gen);
			try_eat(gen, TOKEN_CLOSE_PAREN);
		} break;

		case TOKEN_NUMBER:
		{
			emit_number_expression(gen);
		} break;

		case TOKEN_IDENTIFIER:
		{
			if (peek(gen, 1).type == TOKEN_OPEN_PAREN)
			{
				emit_procedure_expression(gen);
			}
			else if (is_assign_op(peek(gen, 1).type))
			{
				emit_assign_expression(gen);
			}
			else
			{
				emit_identifier_expression(gen);
			}
		} break;

		case TOKEN_TRUE ... TOKEN_FALSE:
		{
			emit_boolean_expression(gen);
		} break;

		default:
		{
			error(gen, "Expected expression, got %s", to_string(expr.type));
		} break;
	}

	if (is_binary_op(peek(gen).type))
	{
		emit_binary_expression(gen);
	}
}

//
// Scope
//

inline void begin_scope(Generator* gen, Scope* scope) 
{
	// {
	try_eat(gen, TOKEN_OPEN_BRACE);

	scope->first_inst = gen->instructions->count;
	scope->local_base = gen->variables.count;

	gen->current_scope = scope;
}

inline void end_scope(Generator* gen, Scope* scope) 
{
	// }
	try_eat(gen, TOKEN_CLOSE_BRACE);

	scope->last_inst = gen->instructions->count;

	trim_end(&gen->variables, scope->local_base);

	gen->current_scope = nullptr;
}

inline void emit_scope(Generator* gen)
{
	Scope scope = {};

	begin_scope(gen, &scope);

	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	end_scope(gen, &scope);
}

//
// statements
//

// var <ident> = <expr>
inline void emit_var_statement(Generator* gen)
{
	// var
	eat(gen);
	
	// <ident>
	Token ident = try_eat(gen, TOKEN_IDENTIFIER);

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

	if (gen->current_procedure && gen->current_scope == &gen->current_procedure->scope) 
	{
		gen->current_procedure->local_count++;
	}
	else if (!gen->current_procedure)
	{
		gen->top_level_var_count++;
	}

	// =
	try_eat(gen, TOKEN_EQUALS);

	// <expr>
	emit_expression(gen);

	emplace(gen->instructions, INSTRUCTION_STORE_LOCAL, get_or_create_variable(gen, name));
}

// define <ident>(<ident>, <ident>...) { ... }
inline void emit_define_statement(Generator* gen)
{
	// define
	eat(gen);

	if (gen->current_procedure)
	{
		error(gen, "Cannot define procedure inside another one");
	}

	// <ident>
	Token ident = try_eat(gen, TOKEN_IDENTIFIER);

	Identifier name = {};
	as_string(ident, gen->source, name, 256);

	if (variable_exists(gen, name) || procedure_exists(gen, name)) 
	{
		error(gen, "Identifier '%s' redefined", name);
	}

	Procedure* proc = get_or_create_procedure(gen, name);

	// (
	Token open = try_eat(gen, TOKEN_OPEN_PAREN);

	// <ident>, <ident>...
	while (peek(gen).type != TOKEN_CLOSE_PAREN)
	{
		// check for (,<ident>
		if (peek(gen, -1).type == TOKEN_OPEN_PAREN && peek(gen).type == TOKEN_COMMA)
		{
			error(gen, "Expected argument after `(` in procedure definition, got `,`");
		}

		// <ident>
		Token ident = try_eat(gen, TOKEN_IDENTIFIER);

		Identifier name = {};
		as_string(ident, gen->source, name, 256);

		if (variable_exists(gen, name))
		{
			error(gen, "Argument/variable redefinition in procedure definition");
		}

		get_or_create_variable(gen, name);

		proc->arg_count++;

		if (peek(gen).type != TOKEN_CLOSE_PAREN)
		{
			// ,
			try_eat(gen, TOKEN_COMMA);
		
			// Check for <ident>,)
			if (peek(gen).type == TOKEN_CLOSE_PAREN)
			{
				error(gen, "Expected argument after `,` in procedure definition, got `)`");
			}
		}
	}

	// )
	try_eat(gen, TOKEN_CLOSE_PAREN);

	// skip the function body
	Instruction* jump = emplace(gen->instructions, INSTRUCTION_JUMP);

	// {
	begin_scope(gen, &proc->scope);

	// move the local index back to the first argument
	proc->scope.local_base = gen->variables.count - proc->arg_count;

	gen->current_procedure = proc;

	// { ... }
	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	// if we didn't emit a `return`, return nothing. 
	if (!proc->returns_value)
	{
		emplace(gen->instructions, INSTRUCTION_LOAD_NULL);
		emplace(gen->instructions, INSTRUCTION_RETURN);
	}

	// }
	end_scope(gen, &proc->scope);

	gen->current_procedure = nullptr;

	// we know the last instruction now, we'll jump here.
	jump->arg = proc->scope.last_inst;
}

// return <expr>
inline void emit_return_statement(Generator* gen)
{
	if (!gen->current_procedure)
	{
		error(gen, "Cannot use return outside procedure");
	}

	// at some point in the procedure we will return a value that's not UNDEFINED. 
	// NOTE: might be an issue if a value is not returned every path
	gen->current_procedure->returns_value = true;

	// return
	eat(gen);

	// <expr>
	emit_expression(gen);

	emplace(gen->instructions, INSTRUCTION_RETURN);
}

inline void patch_breaks(Generator* gen, const Loop* loop) 
{
	for (uint16_t i = loop->scope.first_inst; i < loop->scope.last_inst; i++) 
	{
		Instruction* it = &(*gen->instructions)[i];

		if (it->type == INSTRUCTION_JUMP && it->arg == (uint16_t)-1) 
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
inline void emit_repeat_statement(Generator* gen)
{
	// repeat
	eat(gen);

	// <expr>
	emit_expression(gen);

	Loop loop = {};
	
	// {
	begin_scope(gen, &loop.scope);

	Identifier name = {};
	__builtin_sprintf(name, "repeat_local_%d", loop.scope.local_base);

	loop.local = get_or_create_variable(gen, name);

	// repeat_local = <expr>
	emplace(gen->instructions, INSTRUCTION_STORE_LOCAL, loop.local);

	// repeat_local >= 0
	emplace(gen->instructions, INSTRUCTION_LOAD_LOCAL, loop.local);
	emplace(gen->instructions, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(0.0f)));
	emplace(gen->instructions, INSTRUCTION_LESS_THAN);

	// true? skip the loop
	Instruction* jump_cond = emplace(gen->instructions, INSTRUCTION_JUMP_COND);

	// repeat_local = repeat_local - 1
	emplace(gen->instructions, INSTRUCTION_LOAD_LOCAL, loop.local);
	emplace(gen->instructions, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(1.0f)));
	emplace(gen->instructions, INSTRUCTION_SUB);
	emplace(gen->instructions, INSTRUCTION_STORE_LOCAL, loop.local);

	gen->current_loop = &loop;

	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	gen->current_loop = nullptr;

	end_scope(gen, &loop.scope);

	emplace(gen->instructions, INSTRUCTION_JUMP, (uint16_t)(loop.scope.first_inst + 2));

	jump_cond->arg = loop.scope.last_inst + 1; // last statement + the jump

	// patch breaks
	patch_breaks(gen, &loop);
}

// repeat until <expr> { ... }
inline void emit_repeat_until_statement(Generator* gen) 
{
	// repeat
	eat(gen);

	// until
	eat(gen);

	Loop loop = {};

	uint16_t start = gen->instructions->count;

	// <expr>
	emit_expression(gen);
	
	// {
	begin_scope(gen, &loop.scope);

	loop.scope.first_inst = start;

	// is <expr> true? Skip the loop
	Instruction* jump_cond = emplace(gen->instructions, INSTRUCTION_JUMP_COND);

	gen->current_loop = &loop;

	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	gen->current_loop = nullptr;

	end_scope(gen, &loop.scope);

	emplace(gen->instructions, INSTRUCTION_JUMP, loop.scope.first_inst);

	jump_cond->arg = loop.scope.last_inst + 1; // last statement + the jump

	// patch breaks
	patch_breaks(gen, &loop);
}

// forever { ... }
inline void emit_forever_statement(Generator* gen)
{
	// forever
	eat(gen);	

	Loop loop = {};

	// {
	begin_scope(gen, &loop.scope);
	
	gen->current_loop = &loop;

	// statements
	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	gen->current_loop = nullptr;

	// }
	end_scope(gen, &loop.scope);

	emplace(gen->instructions, INSTRUCTION_JUMP, loop.scope.first_inst);

	patch_breaks(gen, &loop);
}

inline void emit_else_if_statement(Generator* gen);

inline void emit_else_statement(Generator* gen);

// if <expr> { ... }
inline void emit_if_statement(Generator* gen)
{
	// if
	eat(gen);

	// <expr>
	emit_expression(gen);

	// <expr> == false?
	emplace(gen->instructions, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(false)));
	emplace(gen->instructions, INSTRUCTION_EQUALS);

	// true? skip the block.
	Instruction* jump_cond = emplace(gen->instructions, INSTRUCTION_JUMP_COND);

	Scope scope = {};

	// {
	begin_scope(gen, &scope);

	// Statements
	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}
 
	// }
	end_scope(gen, &scope);

	// this jumps to the end of this block if <expr> == false (therefore, not executing this block).
	jump_cond->arg = scope.last_inst;

	if (peek(gen).type == TOKEN_ELSE)
	{
		// these emit an additional jump at the beginning, in case this block ends up executing, so jump here instead.
		if (peek(gen, 1).type == TOKEN_IF)
		{
			emit_else_if_statement(gen);
			jump_cond->arg++;
		}
		else
		{
			emit_else_statement(gen);
			jump_cond->arg++;
		}
	}
}

// else if <expr> { ... }
inline void emit_else_if_statement(Generator* gen) 
{
	// if the last block executes, this will skip this block.
	Instruction* jump = emplace(gen->instructions, INSTRUCTION_JUMP);

	// else if
	eat(gen);
	eat(gen);

	// <expr>
	emit_expression(gen);

	// <expr> == false?
	emplace(gen->instructions, INSTRUCTION_LOAD_IMMEDIATE, get_or_create_immediate(gen, make_value(false)));
	emplace(gen->instructions, INSTRUCTION_EQUALS);

	// true? skip the block.
	Instruction* jump_cond = emplace(gen->instructions, INSTRUCTION_JUMP_COND);

	Scope scope = {};

	// {
	begin_scope(gen, &scope);

	// Statements
	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	// }
	end_scope(gen, &scope);

	// this jumps to the end of this block if the last block was executed. 
	jump->arg = scope.last_inst;

	// this jumps to the end of this block if <expr> == false (therefore, not executing this block).
	jump_cond->arg = scope.last_inst;

	if (peek(gen).type == TOKEN_ELSE)
	{
		// these emit an additional jump at the beginning, in case this block ends up executing, so jump here instead.
		if (peek(gen, 1).type == TOKEN_IF)
		{
			emit_else_if_statement(gen);
			jump_cond->arg++;
		}
		else
		{
			emit_else_statement(gen);
			jump_cond->arg++;
		}
	}
}

// else { ... }
inline void emit_else_statement(Generator* gen) 
{
	Instruction* jump = emplace(gen->instructions, INSTRUCTION_JUMP);

	// else
	eat(gen);

	Scope scope = {};
	
	// {
	begin_scope(gen, &scope);

	// Statements
	while (peek(gen).type != TOKEN_CLOSE_BRACE)
	{
		emit_statement(gen);
	}

	// }
	end_scope(gen, &scope);

	jump->arg = scope.last_inst;
}

// break
inline void emit_break_statement(Generator* gen) 
{
	if (!gen->current_loop)
	{
		error(gen, "Cannot use break outside a loop.");
	}

	// break
	eat(gen);

	// argument is -1 so we know what instructions to patch in the loop statements.
	emplace(gen->instructions, INSTRUCTION_JUMP, (uint16_t)-1);
}

// foreign define <ident>(...)
inline void emit_foreign_define_statement(Generator* gen)
{
	// foreign
	eat(gen);

	// define
	Token define = try_eat(gen, TOKEN_DEFINE);

	// <ident>
	Token ident = try_eat(gen, TOKEN_IDENTIFIER);

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

	Procedure* proc = get_or_create_procedure(gen, name);

	proc->foreign = true;

	// (
	try_eat(gen, TOKEN_OPEN_PAREN);

	// <ident>, <ident>...
	while (peek(gen).type != TOKEN_CLOSE_PAREN)
	{
		// Check for (,<ident>
		if (peek(gen, -1).type == TOKEN_OPEN_PAREN && peek(gen).type == TOKEN_COMMA)
		{
			error(gen, "Expected expression after `(` in foreign procedure declaration, got `,`");
		}

		// <ident>
		try_eat(gen, TOKEN_IDENTIFIER);

		proc->arg_count++;

		if (peek(gen).type != TOKEN_CLOSE_PAREN)
		{
			// ,
			try_eat(gen, TOKEN_COMMA);
	
			// Check for <ident>,)
			if (peek(gen).type == TOKEN_CLOSE_PAREN)
			{
				error(gen, "Expected expression after `,` in foreign procedure declaration, got `)`");
			}
		}
	}

	// )
	try_eat(gen, TOKEN_CLOSE_PAREN);
}

inline void emit_statement(Generator* gen)
{
	switch (peek(gen).type)
	{
		// var <ident> = <expr>
		case TOKEN_VAR:
		{
			emit_var_statement(gen);
		} break;

		case TOKEN_IDENTIFIER:
		{
			// <ident>(...)
			if (peek(gen, 1).type == TOKEN_OPEN_PAREN)
			{
				emit_procedure_expression(gen);
			}
			// <ident> = <expr>
			else if (is_assign_op(peek(gen, 1).type))			
			{
				emit_assign_expression(gen);
				pop(gen->instructions); // pop the LOAD_IMMEDIATE if this is a statement.
			}
		} break;

		// foreign define <ident>(...)
		case TOKEN_FOREIGN:
		{
			emit_foreign_define_statement(gen);
		} break;

		// define <ident>(...) { ... }
		case TOKEN_DEFINE:
		{
			emit_define_statement(gen);
		} break;

		// return <expr>
		case TOKEN_RETURN:
		{
			emit_return_statement(gen);
		} break;

		case TOKEN_REPEAT:
		{
			// repeat until <expr> { ... }
			if (peek(gen, 1).type == TOKEN_UNTIL)
			{
				emit_repeat_until_statement(gen);
			}
			// repeat <expr> { ... }
			else
			{
				emit_repeat_statement(gen);
			}
		} break;

		// forever { ... }
		case TOKEN_FOREVER:
		{
			emit_forever_statement(gen);
		} break;

		// break
		case TOKEN_BREAK:
		{
			emit_break_statement(gen);
		} break;

		// if <expr> { ... } else if <expr> { ... } else { ... }
		case TOKEN_IF:
		{
			emit_if_statement(gen);
		} break;

		// { ... }
		case TOKEN_OPEN_BRACE:
		{
			emit_scope(gen);
		} break;

		// both of these statements cannot exist without an if statement, so these are
		// handled above. 
		case TOKEN_ELSE:
		{
			if (peek(gen, 1).type == TOKEN_IF)
			{
				error(gen, "Expected if statement before else if.");
			}
			else
			{
				error(gen, "Expected if or else if statement before else.");
			}
		} break;

		default:
		{
			error(gen, "Expected statement, got %s", to_string(peek(gen).type));
		} break;
	}
}

//
// public code
//

void init(Generator* gen, Arena* arena, const char* source, Array <Token> tokens)
{
	gen->source = source;

	gen->tokens = tokens;

	gen->immediates = make_array<Value>(arena, MAX_IMMEDIATES);
	gen->variables = make_array<Identifier>(arena, MAX_LOCALS); 
	gen->procedures = make_array<Procedure>(arena, MAX_PROCEDURES);

	gen->current_procedure = nullptr;
	gen->current_loop = nullptr;
	gen->current_scope = nullptr;
	
	gen->line = 1;
	gen->ptr = 0;
}

void emit(Generator* gen, Array <Instruction>* out)
{
	gen->instructions = out;

	while (gen->ptr < gen->tokens.count)
	{
		emit_statement(gen);
	}

	emplace(gen->instructions, INSTRUCTION_HALT);
}