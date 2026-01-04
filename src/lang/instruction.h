#pragma once

#include <base/common.h>

#define DEFINE_INSTRUCTIONS \
	INST(LOAD_IMMEDIATE) \
	INST(LOAD_LOCAL) \
	INST(STORE_LOCAL) \
	INST(LOAD_PROC) \
	INST(LOAD_NULL) \
	INST(ADD) \
	INST(SUB) \
	INST(DIV) \
	INST(MUL) \
	INST(AND) \
	INST(OR) \
	INST(GREATER_THAN) \
	INST(LESS_THAN) \
	INST(EQUALS) \
	INST(GREATER) \
	INST(LESS) \
	INST(NOT) \
	INST(CALL) \
	INST(RET) \
	INST(JUMP) \
	INST(JUMP_COND) \
	INST(NOOP) \
	INST(HALT)

#undef INST
#define INST(name) INSTRUCTION_##name,

enum InstructionType : uint8_t
{
	DEFINE_INSTRUCTIONS
};

struct Instruction
{
	InstructionType type;
	uint16_t arg;
};

#undef INST
#define INST(name) case INSTRUCTION_##name: return #name;

static inline const char* to_string(const InstructionType type)
{
	switch (type)
	{
		DEFINE_INSTRUCTIONS
	}
	
	return "unknown";
}