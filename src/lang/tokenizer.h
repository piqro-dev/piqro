#pragma once

#include <base/common.h>

#include <base/util.h>

#include <base/array.h>

#include <lang/token.h>

struct Tokenizer
{
public:
	Tokenizer(const char* src);

	template <size_t N>
	void tokenize(Array <Token, N>& tokens);

private:
	Token parse_identifier();

	Token parse_string();

	Token parse_number();

	char peek(uint64_t offset = 0);

	char eat();

private:
	const char* m_src;
	uint64_t m_line;
	uint64_t m_idx;
};