#include <lang/vm.h>

VM::VM(Instruction* instructions, uint16_t instruction_count, Value* immediates, uint16_t immediate_count)
	: m_instructions(instructions), m_instruction_count(instruction_count), m_immediates(immediates), m_immediate_count(immediate_count), m_ic(0) {}

void VM::dump_stack()
{
	println("Stack dump:");

	if (m_stack.count() > 0)
	{
		for (uint16_t i = 0; i < m_stack.count(); i++)
		{
			char v[64] = {};
	
			m_stack[i].as_string(v, 64);
	
			println("\t%s", v);
		}
	}
	else
	{
		println("\t(empty)");
	}
}

VM::Trap VM::execute()
{
	Trap r = SUCCESS;

	const Instruction& inst = m_instructions[m_ic];

	switch (inst.type) 
	{
		case Instruction::LOAD_IMMEDIATE:
		{
			r = LOAD_IMMEDIATE(inst.param);
		} break;
	
		case Instruction::LOAD_VAR:
		{
			r = LOAD_VAR(inst.param);	
		} break;
	
		case Instruction::STORE_VAR:
		{
			r = STORE_VAR(inst.param);	
		} break;

		case Instruction::LOAD_LOCAL:
		{
			r = LOAD_LOCAL(inst.param);	
		} break;
	
		case Instruction::LOAD_NULL:
		{
			r = LOAD_NULL();	
		} break;

		case Instruction::ADD:
		{
			r = ADD();
		} break;

		case Instruction::SUB:
		{
			r = SUB();
		} break;

		case Instruction::DIV:
		{
			r = DIV();
		} break;

		case Instruction::MUL:
		{
			r = MUL();
		} break;

		case Instruction::AND:
		{
			r = AND();
		} break;

		case Instruction::OR:
		{
			r = OR();
		} break;

		case Instruction::GREATER_THAN:
		{
			r = GREATER_THAN();
		} break;

		case Instruction::LESS_THAN:
		{
			r = LESS_THAN();
		} break;

		case Instruction::EQUALS:
		{
			r = EQUALS();
		} break;

		case Instruction::GREATER:
		{
			r = GREATER();
		} break;

		case Instruction::LESS:
		{
			r = LESS();
		} break;

		case Instruction::NOT:
		{
			r = NOT();
		} break;

		case Instruction::CALL:
		{
			r = CALL(inst.param);
		} break;

		case Instruction::RET:
		{
			r = RET();
		} break;

		case Instruction::JUMP:
		{
			r = JUMP(inst.param);
		} break;

		case Instruction::JUMP_COND:
		{
			r = JUMP_COND(inst.param);
		} break;

		case Instruction::NOOP:
		{
			// ... do nothing
		} break;

		default:
		{
			r = ILLEGAL_INSTRUCTION;
		} break;
	}

	if (r == SUCCESS)
	{
		m_ic++;
	}

	return r;
}

Instruction VM::current_instruction()
{
	return m_instructions[m_ic];
}

bool VM::is_done()
{
	return !(m_ic < m_instruction_count);
}

bool VM::check_stack_overflow()
{
	return m_stack.count() >= MAX_STACK_SIZE;
}

bool VM::check_stack_underflow()
{
	return m_stack.count() <= 0;
}

uint16_t VM::ic()
{
	return m_ic;
}

Array <Value, VM::MAX_STACK_SIZE>& VM::stack()
{
	return m_stack;
}

Array <CallFrame, VM::MAX_STACK_SIZE>& VM::call_frames()
{
	return m_call_frames;
}

#define VERIFY_STACK_UNDERFLOW() \
	if (check_stack_underflow()) \
	{ \
		return STACK_UNDERFLOW; \
	}

#define VERIFY_STACK_OVERFLOW() \
	if (check_stack_overflow()) \
	{ \
		return STACK_OVERFLOW; \
	}

VM::Trap VM::LOAD_IMMEDIATE(uint16_t idx)
{
	if (idx > m_immediate_count)
	{
		return OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	m_stack.push(m_immediates[idx]);

	return SUCCESS;
}

VM::Trap VM::LOAD_VAR(uint16_t idx)
{
	if (idx >= MAX_VARIABLES)
	{
		return OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	m_stack.push(m_variables[idx]);

	return SUCCESS;
}

VM::Trap VM::STORE_VAR(uint16_t idx)
{
	if (idx > MAX_VARIABLES)
	{
		return OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	m_variables[idx] = m_stack.pop();

	return SUCCESS;
}

VM::Trap VM::LOAD_LOCAL(uint16_t idx)
{
	VERIFY_STACK_OVERFLOW();

	const CallFrame& cf = *m_call_frames.end();

	if (cf.local_base + idx > m_locals.count())
	{
		return OUT_OF_BOUNDS;
	}

	m_stack.push(m_locals[cf.local_base + idx]);

	return SUCCESS;
}

// TODO: Load proc

VM::Trap VM::LOAD_NULL()
{
	VERIFY_STACK_OVERFLOW();

	m_stack.push(Value());

	return SUCCESS;
}

// these ones are very repetative

#define DEFINE_OPS \
	OP(ADD, +) \
	OP(SUB, -) \
	OP(DIV, /) \
	OP(MUL, *) \
	OP(AND, &&) \
	OP(OR, ||) \
	OP(GREATER_THAN, >=) \
	OP(LESS_THAN, <=) \
	OP(EQUALS, ==) \
	OP(GREATER, >) \
	OP(LESS, <)

#define OP(name, op) \
	VM::Trap VM::name() \
	{ \
		VERIFY_STACK_UNDERFLOW(); \
		\
		Value l = m_stack.pop(); \
		\
		VERIFY_STACK_UNDERFLOW(); \
		\
		Value r = m_stack.pop(); \
		\
		VERIFY_STACK_OVERFLOW(); \
		\
		m_stack.push(l op r); \
		\
		return SUCCESS; \
	}

DEFINE_OPS

VM::Trap VM::NOT()
{
	VERIFY_STACK_UNDERFLOW();

	Value l = m_stack.pop();

	VERIFY_STACK_OVERFLOW();

	m_stack.push(!l);

	return SUCCESS;
}

VM::Trap VM::CALL(uint16_t idx)
{
	if (m_call_frames.count() >= MAX_STACK_SIZE)
	{
		return STACK_OVERFLOW;
	}

	CallFrame& cf = m_call_frames.push();

	cf.return_ic = m_ic;

	VERIFY_STACK_UNDERFLOW();

	cf.arg_count = static_cast<uint16_t>(m_stack.pop().as_number());
	cf.stack_base = m_stack.count();
	cf.local_base = m_locals.count();

	for (uint16_t i = 0; i < cf.arg_count; i++)
	{
		VERIFY_STACK_UNDERFLOW();

		m_locals.push(m_stack.pop());
	}

	m_ic = idx - 1;

	return SUCCESS;
}

VM::Trap VM::RET()
{
	const CallFrame& cf = m_call_frames.pop();

	Value top = m_stack.end();

	m_stack.trim_end(cf.stack_base - 1);
	m_locals.trim_end(cf.local_base);

	// push the top of the stack if it isn't undefined
	if (top.type() != Value::UNDEFINED)
	{
		m_stack.push(top);
	}

	m_ic = cf.return_ic;

	return SUCCESS;
}

VM::Trap VM::JUMP(uint16_t to)
{
	if (to > m_instruction_count)
	{
		return OUT_OF_BOUNDS;
	}

	m_ic = to - 1;

	return SUCCESS;
}

VM::Trap VM::JUMP_COND(uint16_t to)
{
	if (to > m_instruction_count)
	{
		return OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	Value v = m_stack.pop();

	if (v.as_boolean())
	{
		m_ic = to - 1;
	}	

	return SUCCESS;
}