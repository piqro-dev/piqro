#include <block/block.h>

struct BlockPool
{
	void init()
	{
		m_first_free = BlockHandle::INVALID;
		m_count = 0;
	}

	Block& get(BlockHandle h)
	{
		return m_elements[h].b;
	}

	BlockHandle make(const Opcode& op)
	{
		BlockHandle h;
	
		if (m_first_free == BlockHandle::INVALID)
		{
			h = static_cast<BlockHandle>(m_count++);
		}
		else
		{
			h = m_first_free;
			m_first_free = m_elements[h].next_free;
		}
	
		Block& b = get(h);
	
		b.m_op = op;
		
		b.m_handle = h;
		b.m_parent = BlockHandle::INVALID;
		b.m_next = BlockHandle::INVALID;
		b.m_prev = BlockHandle::INVALID;
		b.m_first_child = BlockHandle::INVALID;
		b.m_last_child = BlockHandle::INVALID;
	
		return h;
	}

	void destroy(BlockHandle h)
	{
		const Block& b = get(h);
	
		// check siblings if we have a parent
		if (b.m_parent != BlockHandle::INVALID)
		{
			Block& parent = get(b.m_parent);
	
			if (parent.m_first_child == h)
			{
				parent.m_first_child = b.m_next;
			}
	
			if (b.m_prev != BlockHandle::INVALID)
			{
				get(b.m_prev).m_next = b.m_next;
			}
	
			if (b.m_next != BlockHandle::INVALID)
			{
				get(b.m_next).m_prev = b.m_prev;
			}
		}
	
		// nuke all children if there are any
		if (b.m_first_child != BlockHandle::INVALID) 
		{
			BlockHandle ch = b.m_first_child;
	
			while (ch != BlockHandle::INVALID)
			{
				destroy(ch);
				ch = get(ch).m_next; 
			}
		}
	
		m_elements[h].next_free = m_first_free;
		m_first_free = b.m_handle;
	}

	struct
	{
		Block b;
		BlockHandle next_free;
	} m_elements[MAX_BLOCKS];

	BlockHandle m_first_free;
	uint16_t m_count;
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

	if (m_first_child == BlockHandle::INVALID)
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
	
	if (m_first_child != BlockHandle::INVALID)
	{
		BlockHandle ch = m_first_child;

		while (ch != BlockHandle::INVALID)
		{
			g_block_pool.get(ch).dump();
			ch = g_block_pool.get(ch).m_next;
		}
	}

	indent--;
}

Block* Block::parent()
{
	return m_parent == BlockHandle::INVALID ? nullptr :&g_block_pool.get(m_parent);
}

Block* Block::prev()
{
	return m_prev == BlockHandle::INVALID ? nullptr :&g_block_pool.get(m_prev);
}

Block* Block::next()
{
	return m_next == BlockHandle::INVALID ? nullptr :&g_block_pool.get(m_next);
}

Block* Block::first_child()
{
	return m_first_child == BlockHandle::INVALID ? nullptr :&g_block_pool.get(m_first_child);
}

Block* Block::last_child()
{
	return m_last_child == BlockHandle::INVALID ? nullptr : &g_block_pool.get(m_last_child);
}

Opcode& Block::op()
{
	return g_block_pool.get(m_handle).m_op;
}

Value& Block::value()
{
	return g_block_pool.get(m_handle).m_op.value();
}