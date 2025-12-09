#pragma once

#include <core/core.h>

#include <block/block.h>

#include <block/instruction.h>

#include <block/vm.h>

struct Compiler
{
public:
	static constexpr uint16_t MAX_IMMEDIATES = 256;

public:
	inline void init()
	{

	}

	// blocks -> instructions and an immediate table

	// instructions and an immediate table -> binary

	inline uint16_t compile(Block* block, Instruction* out, uint16_t n)
	{
		uint16_t len = 0;

		const auto emit = [&](const Instruction& i)
		{
			ASSERT(len < n);

			out[len++] = i;
		};

		const Opcode& op = block->op();

		switch (op.type()) 
		{
			case Opcode::VALUE:
		}

		/*
		if (block->first_child() != nullptr)
		{
			Block* child = block->first_child();

			while (child)
			{
				len += compile(child, out, n);
				ASSERT(len < n);

				child = child->next();
			}
		}*/

		return len;
	}

private:
	struct Variable
	{
		char name[64];
		uint16_t idx;
	};

private:
	Variable& look_up_variable(const char* name)
	{
		for (uint16_t i = 0; i < m_variable_table.count(); i++)
		{
			if (__builtin_strcmp(name, m_variable_table[i].name) == 0)
			{
				return m_variable_table[i];
			}
		}

		ASSERT(false && "Variable doesn't exist");
	}

private:

	Array <Value, MAX_IMMEDIATES> m_immediates;
	Array <Variable, VM::MAX_VARIABLES> m_variable_table;
};