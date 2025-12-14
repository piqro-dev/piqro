#include <block/block.h>

#include <base/free_list.h>

struct BlockPool
{
public:
	inline void init()
	{
		m_blocks.init();
	}

	inline Block& get(BlockId id)
	{
		return m_blocks[id];
	}

	inline BlockId make(const Opcode& op)
	{
		BlockId id = static_cast<BlockId>(m_blocks.push());

		Block& b = get(id);
	
		b.m_op = op;
		
		b.m_handle = id;
		b.m_parent = BlockId::INVALID;
		b.m_next = BlockId::INVALID;
		b.m_prev = BlockId::INVALID;
		b.m_first_child = BlockId::INVALID;
		b.m_last_child = BlockId::INVALID;
	
		return id;
	}

	inline void destroy(BlockId id)
	{
		const Block& b = get(id);
	
		// check siblings if we have a parent
		if (b.m_parent != BlockId::INVALID)
		{
			Block& parent = get(b.m_parent);
	
			if (parent.m_first_child == id)
			{
				parent.m_first_child = b.m_next;
			}
	
			if (b.m_prev != BlockId::INVALID)
			{
				get(b.m_prev).m_next = b.m_next;
			}
	
			if (b.m_next != BlockId::INVALID)
			{
				get(b.m_next).m_prev = b.m_prev;
			}
		}
	
		// nuke all children if there are any
		if (b.m_first_child != BlockId::INVALID) 
		{
			BlockId ch = b.m_first_child;
	
			while (ch != BlockId::INVALID)
			{
				destroy(ch);
				ch = get(ch).m_next; 
			}
		}
	
		m_blocks.remove(id);
	}

private:
	FreeList <Block, MAX_BLOCKS> m_blocks;
};

static BlockPool g_block_pool;

Block* Block::make(Opcode::OpcodeType type, Value v)
{
	DO_ONCE 
	{
		g_block_pool.init();
	}

	return &g_block_pool.get(g_block_pool.make(Opcode(type, v)));
}

void Block::destroy()
{
	g_block_pool.destroy(m_handle);
}

void Block::append_child(Block* child)
{
	child->m_parent = m_handle;

	if (m_first_child == BlockId::INVALID)
	{
		m_first_child = child->m_handle;
	}
	else
	{	
		Block& last = g_block_pool.get(m_last_child);
	
		last.m_next = child->m_handle;
		child->m_prev = m_last_child;
	}

	m_last_child = child->m_handle;
}

void Block::dump()
{
	static uint32_t indent = 0;

	char a[128] = {};

	for (uint32_t i = 0; i < indent; i++)
	{
		a[i] = ' ';
	}

	a[indent] = '\0';

	if (m_op.is_value())
	{
		char v[64] = {};
		m_op.value().as_string(v, 64);

		println("- %sblock <%s, '%s'>", a, m_op.to_string(), v);
	}
	else
	{
		println("- %sblock <%s>", a, m_op.to_string());
	}

	indent++;
	
	if (m_first_child != BlockId::INVALID)
	{
		BlockId ch = m_first_child;

		while (ch != BlockId::INVALID)
		{
			g_block_pool.get(ch).dump();
			ch = g_block_pool.get(ch).m_next;
		}
	}

	indent--;
}

Block* Block::parent()
{
	return m_parent == BlockId::INVALID ? nullptr : &g_block_pool.get(m_parent);
}

Block* Block::prev()
{
	return m_prev == BlockId::INVALID ? nullptr : &g_block_pool.get(m_prev);
}

Block* Block::next()
{
	return m_next == BlockId::INVALID ? nullptr : &g_block_pool.get(m_next);
}

Block* Block::first_child()
{
	return m_first_child == BlockId::INVALID ? nullptr : &g_block_pool.get(m_first_child);
}

Block* Block::last_child()
{
	return m_last_child == BlockId::INVALID ? nullptr : &g_block_pool.get(m_last_child);
}

Opcode& Block::op()
{
	return g_block_pool.get(m_handle).m_op;
}

Value& Block::value()
{
	return g_block_pool.get(m_handle).m_op.value();
}

BlockId Block::handle() const
{
	return m_handle;
}