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
	char name[64];

	uint8_t arg_count;
	bool returns_value;
	
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

	Array <Instruction>* instructions;

	Scope* current_scope;
	Procedure* current_procedure;
	Loop* current_loop;

	uint16_t line;	
	uint16_t ptr;
};

void init(Generator* gen, Arena* arena, const char* source, Array <Token> token);

void emit_program(Generator* gen, Array <Instruction>* out);