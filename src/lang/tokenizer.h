#pragma once

#include <base/common.h>

#include <base/util.h>

#include <lang/token.h>

struct Tokenizer
{
	const char* source;
	uint16_t line;
	uint32_t ptr;
};

static inline void init(Tokenizer* tok, const char* source);

static inline void tokenize(Tokenizer* tok, Array <Token>* out);