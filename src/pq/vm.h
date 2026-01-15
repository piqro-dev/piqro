#pragma once

#include <base/common.h>

#include <base/arena.h>

#include <pq/types.h>

typedef struct PQ_VM PQ_VM;

typedef struct 
{
	uint16_t return_ip;
	uint16_t stack_base;
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

	String foreign_name;
	PQ_NativeProcedure proc;
} PQ_ProcedureInfo;

typedef void (*PQ_VMErrorFn)(const char*);

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

	bool halt;

	uint16_t ip;
	uint16_t bp;
};

void pq_init_vm(PQ_VM* vm, Arena* arena, const PQ_CompiledBlob* b, PQ_VMErrorFn error);

bool pq_execute(PQ_VM* vm);

PQ_Value pq_get_local(PQ_VM* vm, uint8_t index);

PQ_Value pq_pop(PQ_VM* vm);

void pq_push(PQ_VM* vm, PQ_Value v);

#define pq_return(vm) pq_push((vm), pq_value_null());

#define pq_return_value(vm, ...) pq_push((vm), (__VA_ARGS__))

void pq_bind_foreign_proc(PQ_VM* vm, String name, PQ_NativeProcedure proc);