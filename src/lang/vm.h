#pragma once

#include <base/common.h>

#include <base/array.h>

#include <base/arena.h>

#include <lang/types.h>

#include <lang/config.h>

struct VM;

struct CallFrame
{
	uint16_t return_ip;

	uint16_t stack_base;
	uint16_t local_base;

	uint16_t local_count;
	uint16_t arg_count;
};

typedef void (*NativeProcedure)(VM* vm);

struct ProcedureInfo
{
	bool foreign;

	uint8_t local_count;
	uint8_t arg_count;

	uint16_t first_inst;

	// only used if foreign is true.
	NativeProcedure proc;
};

enum Trap : uint8_t
{
	TRAP_SUCCESS,
	TRAP_STACK_OVERFLOW,
	TRAP_STACK_UNDERFLOW,
	TRAP_OUT_OF_BOUNDS,
	TRAP_ILLEGAL_INSTRUCTION,
	TRAP_HALT_EXECUTION,
	TRAP_UNDEFINED_FOREIGN,
};

inline const char* to_string(Trap trap)
{
	switch (trap)
	{
		case TRAP_SUCCESS: return "Success";
		case TRAP_STACK_OVERFLOW: return "Stack overflow";
		case TRAP_STACK_UNDERFLOW: return "Stack underflow";
		case TRAP_OUT_OF_BOUNDS: return "Out of bounds";
		case TRAP_ILLEGAL_INSTRUCTION: return "Illegal instruction";
		case TRAP_HALT_EXECUTION: return "Execution halted";
		case TRAP_UNDEFINED_FOREIGN: return "Undefined foreign";
	}

	return "unknown";
}

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

Value get_local(VM* vm, uint8_t index);

void bind_procedure(VM* vm, uint8_t index, NativeProcedure proc);