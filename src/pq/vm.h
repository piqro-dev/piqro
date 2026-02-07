#pragma once

#include <base/common.h>
#include <base/arena.h>

#include <pq/types.h>
#include <pq/config.h>

typedef struct PQ_VM PQ_VM;

typedef struct PQ_CallFrame PQ_CallFrame;
struct PQ_CallFrame
{
	uint16_t return_ip;

	uint16_t stack_base;
	uint16_t local_base;

	Scratch scratch;
};

typedef void (*PQ_NativeProcedure)(PQ_VM* vm);

typedef struct PQ_ProcedureInfo PQ_ProcedureInfo;
struct PQ_ProcedureInfo
{
	bool foreign;

	uint16_t local_count;
	uint16_t arg_count;

	uint16_t first_inst;

	String foreign_name;
	PQ_NativeProcedure proc;
};

typedef void (*PQ_VMErrorFn)(const char*);

struct PQ_VM 
{
	Arena* arena;

	PQ_VMErrorFn error;

	PQ_Value* immediates;
	uint16_t immediate_count;

	PQ_ProcedureInfo* proc_infos;
	uint16_t proc_info_count;

	PQ_Instruction* instructions;
	uint16_t instruction_count;

	PQ_CallFrame* call_frames;
	uint16_t call_frame_count;

	PQ_Value* stack;
	uint16_t stack_size;

	PQ_Value* locals;
	uint16_t local_count;

	PQ_Value* globals;
	uint16_t global_count;

	bool halt;

	uint16_t ip;
	uint16_t bp;
};

void pq_vm_init(PQ_VM* vm, Arena* arena, const PQ_CompiledBlob* b, PQ_VMErrorFn error);

bool pq_execute(PQ_VM* vm);

PQ_Value pq_vm_get_local(PQ_VM* vm, uint16_t index);

PQ_Value pq_vm_pop(PQ_VM* vm);

void pq_vm_push(PQ_VM* vm, PQ_Value v);

void pq_vm_bind_foreign_proc(PQ_VM* vm, String name, PQ_NativeProcedure proc);

//
// helpers
//

#define pq_vm_return(vm) pq_vm_push((vm), pq_value_null());

#define pq_vm_return_value(vm, ...) pq_vm_push((vm), (__VA_ARGS__))