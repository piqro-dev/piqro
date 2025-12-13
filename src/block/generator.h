#pragma once

#include <base/common.h>

#include <block/block.h>

#include <block/instruction.h>

#include <block/vm.h>

struct Variable
{
	char name[64];
	uint16_t idx;
};

struct Generator
{
public:
	static constexpr uint16_t MAX_IMMEDIATES = 256;

public:
	template <size_t N>
	void emit_value(Block* block, Array <Instruction, N>& out);

	template <size_t N>
	void emit_expression(Block* block, Array <Instruction, N>& out);

	template <size_t N>
	void emit_get_variable(Block* block, Array <Instruction, N>& out);

	template <size_t N>
	void emit_set_variable(Block* block, Array <Instruction, N>& out);

	template <size_t N>
	void emit_binary_op(Block* block, Array <Instruction, N>& out);

	Array <Value, MAX_IMMEDIATES>& immediates();

	Array <Variable, VM::MAX_VARIABLES>& variables();

private:
	Variable& get_or_create_variable(const char* name);

	Value& get_or_create_immediate(const Value& v, uint16_t& idx);

private:
	Array <Value, MAX_IMMEDIATES> m_immediates;
	Array <Variable, VM::MAX_VARIABLES> m_variables;
};
