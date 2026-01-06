#pragma once

#include <base/common.h>

#include <base/array.h>

#include <base/arena.h>

#include <lang/instruction.h>

#include <lang/value.h>

#include <lang/config.h>

struct CallFrame
{
	uint16_t return_ic;
	uint8_t arg_count;
	uint16_t stack_base;
	uint16_t local_base;
};

enum Trap : uint8_t
{
	TRAP_SUCCESS,
	TRAP_STACK_OVERFLOW,
	TRAP_STACK_UNDERFLOW,
	TRAP_OUT_OF_BOUNDS,
	TRAP_ILLEGAL_INSTRUCTION,
	TRAP_HALT_EXECUTION,
};

struct VM
{
	Array <Instruction> instructions;
	Array <Value> immediates;
	Array <Value> locals;
	Array <Value> stack;
	Array <CallFrame> call_frames;

	uint16_t ic;
};

void init(VM* vm, Arena* arena, Array <Instruction> instructions, Array <Value> immediates);

Trap execute(VM* vm);