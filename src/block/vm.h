#pragma once

#include <base/common.h>

#include <base/array.h>

#include <block/instruction.h>

#include <block/value.h>

struct CallFrame
{
	uint16_t return_ic;
	uint16_t arg_count;
	uint16_t stack_base;
	uint16_t local_base;
};

struct VM
{
public:
	static constexpr uint16_t MAX_STACK_SIZE = 128;

	static constexpr uint16_t MAX_VARIABLES = 64;

	enum Trap
	{
		SUCCESS,
		STACK_OVERFLOW,
		STACK_UNDERFLOW,
		OUT_OF_BOUNDS,
		ILLEGAL_INSTRUCTION,
	};

public:
	void init(Instruction* instructions, uint16_t instruction_count, Value* immediates, uint16_t immediate_count);

	void dump_stack();

	void dump_instructions();

	Trap execute();

	Instruction current_instruction();

	bool is_done();

	bool check_stack_overflow();

	bool check_stack_underflow();

	uint16_t ic();

	Array <Value, MAX_STACK_SIZE>& stack();

	Array <CallFrame, MAX_STACK_SIZE>& call_frames();

private:
	Trap LOAD_IMMEDIATE(uint16_t idx);

	Trap LOAD_VAR(uint16_t idx);

	Trap STORE_VAR(uint16_t idx);

	Trap LOAD_LOCAL(uint16_t idx);

	Trap LOAD_PROC(uint16_t idx);

	Trap LOAD_NULL();

	Trap ADD();

	Trap SUB();

	Trap DIV();

	Trap MUL();

	Trap AND();

	Trap OR();

	Trap GREATER_THAN();

	Trap LESS_THAN();

	Trap EQUALS();

	Trap GREATER();

	Trap LESS();

	Trap NOT();

	Trap CALL(uint16_t arg_count);

	Trap RET();

	Trap JUMP(uint16_t to);

	Trap JUMP_COND(uint16_t to);

private:
	Instruction* m_instructions;
	uint16_t m_instruction_count;

	Value* m_immediates;
	uint16_t m_immediate_count;

	Value m_variables[MAX_VARIABLES];

	Array <Value, MAX_VARIABLES> m_locals;
	Array <Value, MAX_STACK_SIZE> m_stack;
	Array <CallFrame, MAX_STACK_SIZE> m_call_frames;

	uint16_t m_ic;
};
