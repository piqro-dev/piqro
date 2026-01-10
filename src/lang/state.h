#pragma once

#include <base/common.h>

#include <base/arena.h>

#include <lang/compiler.h>

#include <lang/vm.h>

struct State
{
	const char* source;

	Array <uint8_t> blob;

	Compiler com;
	VM vm;
};

void init(State* state, Arena* arena, const char* source);

void bind_procedure(State* state, const char* name, NativeProcedure proc);

void evaluate(State* state);
