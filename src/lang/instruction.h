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

#define INST(name) name,

struct Instruction
{
	enum InstructionType : uint8_t
	{
		DEFINE_INSTRUCTIONS
	};

	inline Instruction() = default;

	inline Instruction(InstructionType type, uint16_t param = 0)
		: type(type), param(param) {}

	#undef INST
	#define INST(name) case name: return #name;

	inline const char* as_string() const
	{
		switch (this->type)
		{
			DEFINE_INSTRUCTIONS
		}

		return "Unknown";
	}

	InstructionType type;
	uint16_t param;
};
