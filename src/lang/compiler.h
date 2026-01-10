#pragma once

#include <base/common.h>

#include <lang/tokenizer.h>

#include <lang/generator.h>

struct Compiler 
{
	const char* source;

	Tokenizer tok;	
	Generator gen;

	Array <Token> tokens;
	Array <Instruction> instructions;
	
	Array <uint8_t>* blob;
};

void init(Compiler* com, Arena* arena, const char* source);

void compile(Compiler* com, Array <uint8_t>* out);