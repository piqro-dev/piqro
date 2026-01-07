#pragma once

struct Compiler 
{
	const char* source;

	Tokenizer tok;
	Generator gen;
};

void init(Compiler* com, Arena* arena, const char* source);

void 