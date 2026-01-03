#include <lang/generator.h>

static inline bool identifier_too_long(Token t)
{
	ASSERT(t.type == Token::IDENTIFIER);

	return t.end - t.start - 1 > MAX_IDENTIFIER_NAME_LENGTH;
}

void Generator::init(const char* source_code, const Token* tokens, uint16_t token_count)
{
	m_source_code = source_code; 
	m_tokens = tokens;
	m_token_count = token_count;

	m_idx = 0;
}

Array <Value, MAX_IMMEDIATES>& Generator::immediates()
{
	return m_immediates;	
}

Array <Identifier, MAX_VARIABLES>& Generator::variables()
{
	return m_variables;
}

template <size_t N>
void Generator::emit_program(Array <Instruction, N>& instructions)
{
	while (m_idx < m_token_count)
	{
		emit_statement(instructions);
	}

	instructions.emplace(Instruction::HALT);
}

Token Generator::peek(int16_t offset)
{
	if (m_idx + offset > m_token_count)
	{
		return { Token::UNKNOWN };
	}

	return m_tokens[m_idx + offset];
}

Token Generator::eat()
{
	return m_tokens[m_idx++];
}

Token Generator::try_eat(Token::TokenType expected)
{
	if (peek().type != expected)
	{
		errorln("Error: Expected %s near %s, got %s", Token{ expected }.as_string(), peek(-1).as_string(), peek().as_string());
	}

	return eat();
}

bool Generator::variable_exists(const char* name)
{
	for (uint16_t i = 0; i < m_variables.count(); i++)
	{
		if (__builtin_strcmp(m_variables[i], name) == 0)
		{
			return true;
		}
	}

	return false;
}

bool Generator::procedure_exists(const char* name)
{
	for (uint16_t i = 0; i < m_procedures.count(); i++)
	{
		if (__builtin_strcmp(m_procedures[i].name, name) == 0)
		{
			return true;
		}
	}

	return false;
}

Procedure& Generator::get_or_create_procedure(const char* name)
{
	for (uint16_t i = 0; i < m_procedures.count(); i++)
	{
		if (__builtin_strcmp(m_procedures[i].name, name) == 0)
		{
			return m_procedures[i];
		}
	}

	Procedure& proc = m_procedures.push();

	__builtin_strcpy(proc.name, name);

	return proc;
}

uint16_t Generator::get_or_create_variable(const char* name)
{
	for (uint16_t i = 0; i < m_variables.count(); i++)
	{
		if (__builtin_strcmp(m_variables[i], name) == 0)
		{
			return i;
		}
	}

	__builtin_strcpy(m_variables.push(), name);

	return m_variables.count() - 1;
}

uint16_t Generator::get_or_create_immediate(const Value& v)
{
	for (uint16_t i = 0; i < m_immediates.count(); i++)
	{
		if (m_immediates[i].as_number() == v.as_number())
		{
			return i;
		}
	}

	m_immediates.push({ v });

	return m_immediates.count() - 1;
}



template <size_t N>
void Generator::emit_number_expression(Array <Instruction, N>& instructions)
{
	// <expr>
	Token expr = eat();

	// TODO: A number might be longer than this, not sure if it needs attention?
	char buf[256] = {};
	expr.value(m_source_code, buf, 256);

	instructions.emplace(Instruction::LOAD_IMMEDIATE, get_or_create_immediate(static_cast<float>(atof(buf))));
}

template <size_t N>
void Generator::emit_identifier_expression(Array <Instruction, N>& instructions)
{
	// <expr>
	Token expr = eat();

	// Expression can be an identifier
	char name[256] = {};
	expr.value(m_source_code, name, 256);

	// ... which may not exist
	if (!variable_exists(name))
	{
		errorln("Error: Undefined identifier '%s'", name);
	}

	instructions.emplace(Instruction::LOAD_LOCAL, get_or_create_variable(name));
}

template <size_t N>
void Generator::emit_boolean_expression(Array <Instruction, N>& instructions)
{
	// <expr>
	Token expr = eat();

	if (expr.type == Token::TRUE)
	{
		instructions.emplace(Instruction::LOAD_IMMEDIATE, get_or_create_immediate(true));
	}
	else if (expr.type == Token::FALSE)
	{
		instructions.emplace(Instruction::LOAD_IMMEDIATE, get_or_create_immediate(false));
	}
}

// <expr>
template <size_t N>
void Generator::emit_expression(Array <Instruction, N>& instructions)
{
	// <expr>
	Token expr = peek();

	if (expr.type == Token::NUMBER_LIT)
	{
		emit_number_expression(instructions);
	}
	else if (expr.type == Token::IDENTIFIER)
	{
		emit_identifier_expression(instructions);
	}
	else if (expr.type == Token::TRUE || expr.type == Token::FALSE)
	{
		emit_boolean_expression(instructions);
	}
	else
	{
		errorln("Error: Expected expression near %s, got %s", peek(-1).as_string(), expr.as_string());
	}
}

// <ident> = <expr>
template <size_t N>
void Generator::emit_assign_statement(Array <Instruction, N>& instructions)
{
	// <ident>
	Token ident = eat();

	char name[256] = {};
	ident.value(m_source_code, name, 256);

	// It may not exist 
	if (!variable_exists(name))
	{
		errorln("Error: Undefined identifier '%s'", name);
	}

	// =
	eat();

	// <expr>
	emit_expression(instructions);

	instructions.emplace(Instruction::STORE_LOCAL, get_or_create_variable(name));
}

// var <ident> = <expr>
template <size_t N>
void Generator::emit_var_statement(Array <Instruction, N>& instructions)
{
	// var
	Token var = eat();
	
	// <ident>
	Token id = try_eat(Token::IDENTIFIER);

	char name[256] = {};
	id.value(m_source_code, name, 256);

	// Reject anything longer than sizeof (Variable::name)
	if (identifier_too_long(id))
	{
		errorln("Error: Identifier '%s' is too long", name);
	}

	// It might already exist
	if (variable_exists(name))
	{
		errorln("Error: Variable '%s' redefined", name);
	}

	// =
	try_eat(Token::EQUALS);

	// <expr>
	emit_expression(instructions);

	instructions.emplace(Instruction::STORE_LOCAL, get_or_create_variable(name));
}

// FIXME: Nested loops are wacky and don't work.
// Example: repeat 2 { repeat 2 {} } will execute 6 times.

// repeat <expr> {}
template <size_t N>
void Generator::emit_repeat_statement(Array <Instruction, N>& instructions)
{
	// repeat
	Token repeat = eat();

	// <expr>
	emit_expression(instructions);

	// Create a hidden variable and do the logic on it 
	char name[256] = {};
	__builtin_sprintf(name, "_repeat_local_%hu", static_cast<uint16_t>(m_variables.count()));

	// _repeat_local = <expr>
	const uint16_t repeat_var = get_or_create_variable(name);
	instructions.emplace(Instruction::STORE_LOCAL, repeat_var);

	// {}
	const Scope scope = emit_scope(instructions);

	// _repeat_local = _repeat_local - 1
	instructions.emplace(Instruction::LOAD_LOCAL, repeat_var);
	instructions.emplace(Instruction::LOAD_IMMEDIATE, get_or_create_immediate(1.0f));
	instructions.emplace(Instruction::SUB);
	instructions.emplace(Instruction::STORE_LOCAL, repeat_var);

	// If (0 < _repeat_local), jump back 
	instructions.emplace(Instruction::LOAD_IMMEDIATE, get_or_create_immediate(0.0f));
	instructions.emplace(Instruction::LOAD_LOCAL, repeat_var);
	instructions.emplace(Instruction::LESS);
	instructions.emplace(Instruction::JUMP_COND, scope.first_inst);
}

// define <ident>(<ident>, <ident>, ...) { ... }
template <size_t N>
void Generator::emit_define_statement(Array <Instruction, N>& instructions)
{
	if (m_in_procedure)
	{
		errorln("Error: Cannot define procedure inside another one");
	}

	// define
	Token define = eat();

	// <ident>
	Token ident = try_eat(Token::IDENTIFIER);

	Identifier name = {};
	ident.value(m_source_code, name, 256);

	// It might already exist
	if (procedure_exists(name))
	{
		errorln("Error: Variable '%s' redefined", name);
	}

	Procedure& proc = get_or_create_procedure(name);

	// (
	Token open = try_eat(Token::OPEN_BRACE);

	// <ident>, <ident>...
	while (peek().type != Token::CLOSE_BRACE)
	{
		// Check for (,<ident>
		if (peek().type == Token::COMMA && peek(-1).type == Token::OPEN_BRACE)
		{
			errorln("Error: Expected parameter before `)` in procedure definition, got `,`");
		}

		Token ident = try_eat(Token::IDENTIFIER);

		// Check for <ident>,)
		if (peek().type == Token::COMMA)
		{
			eat();

			if (peek().type == Token::CLOSE_BRACE)
			{
				errorln("Error: Expected parameter after `,` in procedure definition, got `)`");
			}
		}
	}

	// )
	Token close = try_eat(Token::CLOSE_BRACE);

	m_in_procedure = true;

	// { ... }
	emit_scope(instructions);

	m_in_procedure = false;
}

// <ident>(<expr>, <expr>, ...)
template <size_t N>
void Generator::emit_procedure_statement(Array <Instruction, N>& instructions)
{
	// <ident>
	Token ident = eat();

	Identifier name = {};
	ident.value(m_source_code, name, 256);

	// Ensure it's short enough
	if (identifier_too_long(ident))
	{
		errorln("Error: Procedure name '%s' is too long", name);
	}

	// May not exist at the time
	if (!procedure_exists(name))
	{
		errorln("Error: Undefined procedure '%s'", name);
	}

	// (
	Token open = eat();

	// <expr>, <expr>...
	while (peek().type != Token::CLOSE_BRACE)
	{
		// Check for (,<expr>
		if (peek().type == Token::COMMA && peek(-1).type == Token::OPEN_BRACE)
		{
			errorln("Error: Expected parameter before `)` in procedure definition, got `,`");
		}

		emit_expression(instructions);

		// Check for <expr>,)
		if (peek().type == Token::COMMA)
		{
			eat();

			if (peek().type == Token::CLOSE_BRACE)
			{
				errorln("Error: Expected parameter after `,` in procedure definition, got `)`");
			}
		}
	}

	// )
	Token close = try_eat(Token::CLOSE_BRACE);
}

template <size_t N>
void Generator::emit_statement(Array <Instruction, N>& instructions)
{
	// var <ident> = ...
	if (peek().type == Token::VAR)
	{
		emit_var_statement(instructions);
	}
	// <ident> = ...
	else if (peek().type == Token::IDENTIFIER && peek(1).type == Token::EQUALS)
	{
		emit_assign_statement(instructions);
	}
	// repeat <expr> {}
	else if (peek().type == Token::REPEAT)
	{
		emit_repeat_statement(instructions);
	}
	// { ... }
	else if (peek().type == Token::OPEN_CURLY)
	{
		emit_scope(instructions);
	}
	// define <ident>(<ident>, <ident>, ...)
	else if (peek().type == Token::DEFINE)
	{
		emit_define_statement(instructions);
	}
	// <ident>(<expr>, <expr>, ...)
	else if (peek().type == Token::IDENTIFIER && peek(1).type == Token::OPEN_BRACE)
	{
		emit_procedure_statement(instructions);
	}
	else
	{
		errorln("Error: Expected statement, got %s", peek().as_string());
	}
}

template <size_t N>
Scope Generator::emit_scope(Array <Instruction, N>& instructions)
{
	// {
	Token open = eat();

	Scope& scope = m_scopes.push();

	scope.first_inst = instructions.count() - 1;
	scope.local_base = m_variables.count();

	// Statements
	while (peek().type != Token::CLOSE_CURLY)
	{
		emit_statement(instructions);
	}

	// }
	Token close = eat();

	if (close.type != Token::CLOSE_CURLY)
	{
		errorln("Error: Expected `}`, got %s", close.as_string());
	}

	m_variables.trim_end(scope.local_base);

	return m_scopes.pop();
}