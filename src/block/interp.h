#pragma once

#include <core/core.h>

#include <block/block.h>

static constexpr uint16_t MAX_VARIABLES = 64;

struct Variable
{
	char name[64];
	Value value;
};

struct Interpreter
{
public:
	void init();

	Value eval(Block* b);

private:
	Value& set_var(const char* name, const Value& v);

	Value& get_var(const char* name);

private:
	Variable m_variables[MAX_VARIABLES];

	uint16_t m_variable_count;
};
