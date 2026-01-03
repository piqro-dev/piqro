#pragma once

#include <base/common.h>

#include <lang/value.h>

#include <lang/instruction.h>

struct Scope
{
	uint16_t first_inst;
	uint16_t local_base;
};

struct Procedure
{
	char name[64];
	uint8_t param_count;
	uint16_t first_inst;
	uint16_t last_inst;
};

struct NativeProcedure
{
	char name[64];
	uint8_t param_count;
	void (*callback)();
};

static constexpr uint16_t MAX_IDENTIFIER_NAME_LENGTH = 128;
static constexpr uint16_t MAX_IMMEDIATES = 128;
static constexpr uint16_t MAX_VARIABLES = 128;
static constexpr uint16_t MAX_PROCEDURES = 64;
static constexpr uint16_t MAX_SCOPES = 32;

using Identifier = char[MAX_IDENTIFIER_NAME_LENGTH];

struct Generator
{
public:

public:
	void init(const char* source_code, const Token* tokens, uint16_t token_count);
	
	Array <Value, MAX_IMMEDIATES>& immediates();

	Array <Identifier, MAX_VARIABLES>& variables();

	template <size_t N>
	void emit_program(Array <Instruction, N>& instructions);

private:
	Token peek(int16_t offset = 0);

	Token eat();

	Token try_eat(Token::TokenType expected);

	Procedure& get_or_create_procedure(const char* name);

	uint16_t get_or_create_variable(const char* name);

	uint16_t get_or_create_immediate(const Value& v);

	//
	// Expressions
	//

	template <size_t N>
	void emit_number_expression(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_identifier_expression(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_boolean_expression(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_expression(Array <Instruction, N>& instructions);

	//
	// Statements
	//

	template <size_t N>
	void emit_assign_statement(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_var_statement(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_repeat_statement(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_define_statement(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_procedure_statement(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_statement(Array <Instruction, N>& instructions);

	//
	// Scope
	//

	template <size_t N>
	Scope emit_scope(Array <Instruction, N>& instructions);

private:
	const char* m_source_code;
	const Token* m_tokens;
	uint16_t m_token_count;

	Array <Value, MAX_IMMEDIATES> m_immediates;
	Array <Identifier, MAX_VARIABLES> m_variables;
	Array <Scope, MAX_SCOPES> m_scopes;
	Array <Procedure, MAX_PROCEDURES> m_procedures;

	bool m_in_procedure;

	uint16_t m_idx;
};
