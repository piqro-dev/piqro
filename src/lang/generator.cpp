#include <lang/generator.h>

static inline bool identifier_too_long(Token t)
{
	ASSERT(t.type == Token::IDENTIFIER);

	return t.end - t.start - 1 > sizeof(Variable::name);
}

Generator::Generator(const char* source_code, const Token* tokens, uint16_t token_count)
	: m_source_code(source_code), m_tokens(tokens), m_token_count(token_count) {}



void Generator::dump_immediates()
{
	println("\nImmediate dump:");

	for (uint16_t i = 0; i < m_immediates.count(); i++)
	{
		char buf[128] = {};
		m_immediates[i].as_string(buf, 128);

		println("\t%d: %s", i, buf);
	}
}

void Generator::dump_variables()
{
	println("\nVariable dump:");

	for (uint16_t i = 0; i < m_variables.count(); i++)
	{
		println("\t%d: %s", i, m_variables[i].name);
	}
}

template <size_t N>
void Generator::emit_number_expression(Array <Instruction, N>& instructions)
{
	// <expr>
	Token expr = eat();

	// TODO: A number might be longer than this, not sure if it needs attention?
	char buf[256] = {};
	expr.value(m_source_code, buf, 256);

	uint16_t idx = 0;
	get_or_create_immediate(static_cast<float>(atof(buf)), idx);

	instructions.emplace(Instruction::LOAD_IMMEDIATE, idx);
}

template <size_t N>
void Generator::emit_identifier_expression(Array <Instruction, N>& instructions)
{
	// <expr>
	Token expr = eat();

	ASSERT(expr.type == Token::IDENTIFIER);

	// Expression can be an identifier
	char name[256] = {};
	expr.value(m_source_code, name, 256);

	// ... which may not exist
	if (!variable_exists(name))
	{
		errorln("Error: Undefined identifier '%s'", name);
	}

	instructions.emplace(Instruction::LOAD_VAR, get_or_create_variable(name).idx);
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
	else
	{
		errorln("Error: Expected expression, got %s", expr.as_string());
	}
}

// <ident> = <expr>
template <size_t N>
void Generator::emit_var_assignment(Array <Instruction, N>& instructions)
{
	// <ident>
	Token ident = eat();

	if (ident.type != Token::IDENTIFIER)
	{
		errorln("Error: Expected identifier, got %s", ident.as_string());
	}

	char name[256] = {};
	ident.value(m_source_code, name, 256);

	// It may not exist 
	if (!variable_exists(name))
	{
		errorln("Error: Undefined identifier '%s'", name);
	}

	// =
	ASSERT(eat().type == Token::EQUALS);

	// <expr>
	emit_expression(instructions);

	instructions.emplace(Instruction::STORE_VAR, get_or_create_variable(name).idx);
}

// var <ident> = <expr>
template <size_t N>
void Generator::emit_var_statement(Array <Instruction, N>& instructions)
{
	// var
	Token var = eat();
	
	// <ident>
	Token id = eat();

	if (id.type != Token::IDENTIFIER)
	{
		errorln("Error: Expected identifier near %s, got %s", var.as_string(), id.as_string());
	}

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
	ASSERT(eat().type == Token::EQUALS);

	// <expr>
	emit_expression(instructions);
	
	instructions.emplace(Instruction::STORE_VAR, get_or_create_variable(name).idx);
}

template <size_t N>
void Generator::emit_program(Array <Instruction, N>& instructions)
{
	while (m_idx < m_token_count)
	{
		// var <ident> = ...
		if (peek().type == Token::VAR && peek(1).type == Token::IDENTIFIER && peek(2).type == Token::EQUALS)
		{
			emit_var_statement(instructions);
		}
		// <ident> = ...
		else if (peek().type == Token::IDENTIFIER && peek(1).type == Token::EQUALS)
		{
			emit_var_assignment(instructions);
		}
		else
		{
			errorln("TODO: Unhandled statement.. Last token: %s", peek().as_string());
		}
	}

	instructions.emplace(Instruction::HALT);
}

Array <Value, Generator::MAX_IMMEDIATES>& Generator::immediates()
{
	return m_immediates;	
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

bool Generator::variable_exists(const char* name)
{
	for (const Variable& v : m_variables)
	{
		if (__builtin_strcmp(name, v.name) == 0)
		{
			return true;
		}
	}

	return false;
}

Variable& Generator::get_or_create_variable(const char* name)
{
	for (Variable& v : m_variables)
	{
		if (__builtin_strcmp(name, v.name) == 0)
		{
			return v;
		}
	}

	Variable& variable = m_variables.push();
	
	__builtin_strcpy(variable.name, name);
	variable.idx = m_variables.count() - 1;

	return variable;
}

Value& Generator::get_or_create_immediate(const Value& v, uint16_t& idx)
{
	for (uint16_t i = 0; i < m_immediates.count(); i++)
	{
		Value& imm = m_immediates[i];

		if (imm.as_number() == v.as_number())
		{
			idx = i;
			return imm;
		}
	}

	// Push to the end in case it doesn't exist

	Value& imm = m_immediates.push({ v });

	idx = m_immediates.count() - 1;

	return imm;
}
