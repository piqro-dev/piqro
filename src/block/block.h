#pragma once

#include <core/core.h>

#include <block/opcode.h>

static constexpr uint16_t MAX_BLOCKS = 512;

enum BlockHandle : uint16_t
{
	INVALID = static_cast<uint16_t>(-1)
};

struct Block
{
	static Block* make(const Opcode& op);

	void destroy();

	void append_child(Block* child);

	void dump();

	Block* parent();

	Block* prev();

	Block* next();

	Block* first_child();

	Block* last_child(); 

	Opcode& op();

	Value& value();

	friend struct BlockManager;

private:
	Opcode m_op;

	BlockHandle m_handle;
	BlockHandle m_parent;
	BlockHandle m_prev;
	BlockHandle m_next;
	BlockHandle m_first_child;
	BlockHandle m_last_child;
};