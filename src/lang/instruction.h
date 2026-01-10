#pragma once

#include <base/common.h>

#define DEFINE_INSTRUCTIONS \
	INST(CALL) \
	INST(LOAD_NULL) \
	INST(LOAD_IMMEDIATE) \
	INST(LOAD_LOCAL) \
	INST(STORE_LOCAL) \
	INST(JUMP) \
	INST(JUMP_COND) \
	INST(ADD) \
	INST(SUB) \
	INST(DIV) \
	INST(MUL) \
	INST(MOD) \
	INST(AND) \
	INST(OR) \
	INST(GREATER_THAN) \
	INST(LESS_THAN) \
	INST(EQUALS) \
	INST(GREATER) \
	INST(LESS) \
	INST(NOT) \
	INST(NEGATE) \
	INST(RETURN) \
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

inline const char* to_string(const InstructionType type)
{
	switch (type)
	{
		DEFINE_INSTRUCTIONS
	}
	
	return "unknown";
}

// the DEFINE_INSTRUCTIONS macro above is sorted in that way so this check is very easily done
inline bool needs_argument(const InstructionType type)
{
	return type >= INSTRUCTION_CALL && type <= INSTRUCTION_JUMP_COND;
}