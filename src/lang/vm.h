#pragma once

#include <base/common.h>

#include <base/array.h>

#include <base/arena.h>

#include <lang/instruction.h>

#include <lang/value.h>

#include <lang/config.h>

struct CallFrame
{
	uint16_t return_ip;

	uint16_t stack_base;
	uint16_t local_base;

	uint16_t local_count;
	uint16_t arg_count;
};

struct ProcedureInfo
{
	bool foreign;

	uint8_t local_count;
	uint8_t arg_count;

	uint16_t first_inst;
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
	uint8_t* blob;
	uint16_t blob_size;

	Array <Value> immediates;
	Array <ProcedureInfo> procedure_infos;
	Array <Instruction> instructions;

	Array <CallFrame> call_frames;
	Array <Value> stack;
	Array <Value> locals;

	// instruction pointer
	uint16_t ip;

	// blob pointer
	uint16_t bp;
};

void init(VM* vm, Arena* arena, uint8_t* blob, uint16_t blob_size);

Trap execute(VM* vm);