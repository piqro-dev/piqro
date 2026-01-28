#pragma once

#include <base/common.h>

#include <pq/types.h>
#include <pq/config.h>

typedef struct PQ_Scope PQ_Scope;
struct PQ_Scope
{
	uint16_t first_inst; 
	uint16_t last_inst;
	uint16_t local_base;

	PQ_Scope* previous; 
};

typedef struct PQ_Procedure PQ_Procedure;
struct PQ_Procedure
{
	String name;
	uint16_t idx;

	uint16_t local_count;
	uint16_t arg_count;

	bool used;
	bool foreign;

	PQ_Scope scope;
};

typedef struct PQ_Variable PQ_Variable;
struct PQ_Variable
{
	String name;
	uint16_t idx;

	bool global;

	bool array;
	uint16_t array_size;
};

typedef struct PQ_Loop PQ_Loop;
struct PQ_Loop
{
	PQ_Scope scope;
};

typedef void (*PQ_CompilerErrorFn)(uint16_t, const char*);

typedef struct PQ_Compiler PQ_Compiler;
struct PQ_Compiler 
{
	Arena* arena;

	String source;

	PQ_CompilerErrorFn error;

	PQ_Token* tokens;
	uint16_t token_count;

	PQ_Instruction* instructions;
	uint16_t instruction_count;

	PQ_Value* immediates;
	uint16_t immediate_count;

	PQ_Procedure* procedures;
	uint16_t procedure_count;

	PQ_Variable* globals;
	uint16_t global_count;

	PQ_Variable* locals;
	uint16_t local_count;
	uint16_t all_local_count;

	PQ_Scope* current_scope;
	PQ_Procedure* current_proc;
	PQ_Loop* current_loop;

	uint16_t line;
	uint16_t idx;
};

void pq_compiler_init(PQ_Compiler* c, Arena* arena, String source, PQ_CompilerErrorFn error);

PQ_CompiledBlob pq_compile(PQ_Compiler* c);

void pq_compiler_declare_foreign_proc(PQ_Compiler* c, String name, uint16_t arg_count);