#pragma once

#include <base/common.h>

#include <pq/types.h>

#include <pq/config.h>

typedef struct
{
	uint16_t first_inst; 
	uint16_t last_inst;  
	uint16_t local_base; 
} PQ_Scope;

typedef struct
{
	String name;
	
	uint8_t idx;

	uint16_t local_count;
	uint16_t arg_count;

	bool foreign;

	PQ_Scope scope;
} PQ_Procedure;

typedef struct
{
	uint16_t local;
	PQ_Scope scope;
} PQ_Loop;

typedef struct 
{
	String name;
	uint8_t idx;

	bool array;
	uint16_t array_size;

	bool arg;
	bool top_level;
} PQ_Variable;

typedef void (*PQ_CompilerErrorFn)(uint16_t, const char*);

typedef struct 
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

	PQ_Variable* variables;
	uint16_t variable_count;

	PQ_Scope* current_scope;
	PQ_Procedure* current_proc;
	PQ_Loop* current_loop;

	uint16_t line;
	uint16_t idx;
} PQ_Compiler;

void pq_init_compiler(PQ_Compiler* c, Arena* arena, String source, PQ_CompilerErrorFn error);

PQ_CompiledBlob pq_compile(PQ_Compiler* c);

void pq_declare_foreign_proc(PQ_Compiler* c, String name, uint8_t arg_count);