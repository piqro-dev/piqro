#pragma once

#include <base/common.h>

#include <block/opcode.h>

static constexpr uint16_t MAX_BLOCKS = 512;

enum BlockId : uint16_t
{
	INVALID = static_cast<uint16_t>(-1)
};

struct Block
{
public:	
	static Block* make(Opcode::OpcodeType type, const Value& v = {});

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

	BlockId handle() const;

private:
	Opcode m_op;

	BlockId m_handle;
	BlockId m_parent;
	BlockId m_prev;
	BlockId m_next;
	BlockId m_first_child;
	BlockId m_last_child;
	
	friend struct BlockPool;
};