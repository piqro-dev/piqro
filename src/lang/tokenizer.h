#pragma once

#include <base/common.h>

#include <base/util.h>

#include <lang/token.h>

struct Tokenizer
{
	const char* source;

	uint16_t line;
	uint16_t ptr;
};

void init(Tokenizer* tok, const char* source);

void tokenize(Tokenizer* tok, Array <Token>* out);