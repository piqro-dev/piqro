#pragma once

#include <base/common.h>

#include <base/array.h>

#include <lang/value.h>

#include <lang/instruction.h>

struct Variable
{
	char name[256];
	uint16_t idx;
};

struct Generator
{
public:
	static constexpr uint16_t MAX_IMMEDIATES = 256;

	static constexpr uint16_t MAX_VARIABLES = 512;

public:
	Generator(const char* source_code, const Token* tokens, uint16_t token_count);

	void dump_immediates();

	void dump_variables();

	template <size_t N>
	void emit_number_expression(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_identifier_expression(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_expression(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_var_assignment(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_var_statement(Array <Instruction, N>& instructions);

	template <size_t N>
	void emit_program(Array <Instruction, N>& instructions);

	Array <Value, MAX_IMMEDIATES>& immediates();

private:
	Token peek(int16_t offset = 0);

	Token eat();

	bool variable_exists(const char* name);

	Variable& get_or_create_variable(const char* name);

	Value& get_or_create_immediate(const Value& v, uint16_t& idx);

private:
	const char* m_source_code;
	const Token* m_tokens;
	uint16_t m_token_count;

	Array <Value, MAX_IMMEDIATES> m_immediates;
	Array <Variable, MAX_VARIABLES> m_variables;
	
	uint16_t m_idx;
};
