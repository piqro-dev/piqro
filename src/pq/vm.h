#pragma once

#include <base/common.h>

#include <base/arena.h>

#include <pq/types.h>

typedef struct PQ_VM PQ_VM;

typedef struct 
{
	uint16_t return_ip;

	uint16_t stack_base;
	uint16_t local_base;

	uint16_t local_count;
	uint16_t arg_count;
} PQ_CallFrame;

typedef void (*PQ_NativeProcedure)(PQ_VM* vm);

typedef struct
{
	bool foreign;
	uint8_t local_count;
	uint8_t arg_count;
	uint16_t first_inst;

	PQ_NativeProcedure proc;
} PQ_ProcedureInfo;

typedef enum : uint8_t
{
	TRAP_SUCCESS,
	TRAP_STACK_OVERFLOW,
	TRAP_STACK_UNDERFLOW,
	TRAP_OUT_OF_BOUNDS,
	TRAP_ILLEGAL_INSTRUCTION,
	TRAP_HALT_EXECUTION,
	TRAP_UNDEFINED_FOREIGN,
} PQ_Trap;

static inline const char* pq_trap_to_c_str(PQ_Trap trap)
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

typedef void (*PQ_VMErrorFn)(const char*, ...);

struct PQ_VM 
{
	Arena* arena;

	PQ_VMErrorFn error;

	PQ_Value* immediates;
	uint8_t immediate_count;

	PQ_ProcedureInfo* proc_infos;
	uint8_t proc_info_count;

	PQ_Instruction* instructions;
	uint16_t instruction_count;

	PQ_CallFrame* call_frames;
	uint8_t call_frame_count;

	PQ_Value* stack;
	uint8_t stack_size;

	PQ_Value* locals;
	uint8_t local_count;

	uint16_t ip;
	uint16_t bp;
};

void pq_vm_init(PQ_VM* vm, Arena* arena, const PQ_CompiledBlob* b, PQ_VMErrorFn error);

PQ_Trap pq_vm_execute(PQ_VM* vm);

PQ_Value pq_vm_get_local(PQ_VM* vm, uint8_t index);

PQ_Value pq_vm_pop(PQ_VM* vm);

void pq_vm_push(PQ_VM* vm, PQ_Value v);

void pq_vm_bind_procedure(PQ_VM* vm, uint8_t index, PQ_NativeProcedure proc);