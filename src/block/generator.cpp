#include <block/generator.h>

template <size_t N>
void Generator::emit_value(Block* block, Array <Instruction, N>& out)
{
	ASSERT(block->op().is_value());

	uint16_t idx = 0;
	get_or_create_immediate(block->value(), idx);

	out.emplace(Instruction::LOAD_IMMEDIATE, idx);
}

template <size_t N>
void Generator::emit_expression(Block* block, Array <Instruction, N>& out)
{	
	const Opcode& op = block->op();

	ASSERT(op.is_expression());

	switch (op.type()) 
	{
		case Opcode::VALUE:
		{
			emit_value(block, out);
		} break;

		case Opcode::GET_VAR:
		{
			emit_get_variable(block, out);
		} break;

		default:
		{
			ASSERT(false && "Unknown opcode");
		} break;
	}
}

template <size_t N>
void Generator::emit_get_variable(Block* block, Array <Instruction, N>& out)
{
	const Opcode& op = block->op();

	ASSERT(op.type() == Opcode::GET_VAR);

	Block* l = block->first_child();

	// TODO: enforce limits, MAX_NAME_LENGTH, etc
	char name[64];
	l->value().as_string(name, 64);
	
	Variable& var = get_or_create_variable(name);

	out.emplace(Instruction::LOAD_VAR, var.idx);
}

template <size_t N>
void Generator::emit_set_variable(Block* block, Array <Instruction, N>& out)
{
	const Opcode& op = block->op();

	ASSERT(op.type() == Opcode::SET_VAR);

	Block* l = block->first_child();

	// TODO: enforce limits, MAX_NAME_LENGTH, etc
	char name[64];
	l->value().as_string(name, 64);
	
	Variable& var = get_or_create_variable(name);

	emit_expression(l->next(), out);
	out.emplace(Instruction::STORE_VAR, var.idx);
}

template <size_t N>
void Generator::emit_binary_op(Block* block, Array <Instruction, N>& out)
{
	const Opcode& op = block->op();

	Block* l = block->first_child();
	Block* r = l->next();

	switch (op.type()) 
	{
		case Opcode::ADD:
		{
			emit_expression(l, out);
			emit_expression(r, out);

			out.emplace(Instruction::ADD);
		} break;

		default:
		{
			ASSERT(false && "Unknown binary op");
		} break;
	}
}

Array <Value, Generator::MAX_IMMEDIATES>& Generator::immediates()
{
	return m_immediates;
}

Array <Variable, VM::MAX_VARIABLES>& Generator::variables()
{
	return m_variables;
}

Variable& Generator::get_or_create_variable(const char* name)
{
	for (uint16_t i = 0; i < m_variables.count(); i++)
	{
		if (__builtin_strcmp(name, m_variables[i].name) == 0)
		{
			return m_variables[i];
		}
	}

	Variable& variable = m_variables.push();
	
	__builtin_strcpy(variable.name, name);
	variable.idx = m_variables.count() - 1;

	return variable;
}

Value& Generator::get_or_create_immediate(const Value& v, uint16_t& idx)
{
	for (uint16_t i = 0; i < m_immediates.count(); i++)
	{
		Value& imm = m_immediates[i];

		if (imm.as_number() == v.as_number())
		{
			idx = i;
			return imm;
		}
	}

	// we push to the end in-case it doesn't exist

	Value& imm = m_immediates.push({ v });

	idx = m_immediates.count() - 1;

	return imm;
}
