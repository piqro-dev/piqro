#pragma once

#include <base/common.h>

#include <base/array.h>

#include <base/arena.h>

#include <lang/value.h>

#include <lang/instruction.h>

#include <lang/config.h>

typedef char Identifier[MAX_IDENTIFIER_NAME_LENGTH];

struct Scope
{
	uint16_t first_inst; 
	uint16_t last_inst;  
	uint16_t local_base; 
};

struct Procedure
{
	Identifier name;

	uint8_t idx;
	uint8_t local_count;
	uint8_t arg_count;

	bool returns_value;
	bool foreign;

	Scope scope;
};

struct Loop
{
	uint16_t local;

	Scope scope;
};

struct Generator
{
	const char* source;
	
	Array <Token> tokens;
	
	Array <Value> immediates;
	Array <Identifier> variables;
	Array <Procedure> procedures;

	Scope* current_scope;
	Procedure* current_procedure;
	Loop* current_loop;

	uint16_t top_level_var_count;

	uint16_t line;	
	uint16_t ptr;

	Array <Instruction>* instructions;
};

void init(Generator* gen, Arena* arena, const char* source, Array <Token> tokens);

void emit_program(Generator* gen, Array <Instruction>* out);