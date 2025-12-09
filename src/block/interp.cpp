#include <block/interp.h>

void Interpreter::init()
{
	m_variable_count = 0;
}

Value& Interpreter::set_var(const char* name, const Value& v)
{
	uint16_t idx = 0;	
	bool found = false;

	for (; idx < m_variable_count; idx++)
	{
		if (__builtin_strcmp(m_variables[idx].name, name) == 0)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		idx = m_variable_count++;
	}
	else
	{
		__builtin_strcpy(m_variables[idx].name, name);
		m_variables[idx].value = v;
		println("%s", m_variables[idx].name);
	}

	return m_variables[idx].value;
}

Value& Interpreter::get_var(const char* name)
{
	uint16_t idx = 0;	
	bool found = false;

	for (; idx < m_variable_count; idx++)
	{
		if (__builtin_strcmp(m_variables[idx].name, name) == 0)
		{
			found = true;
			break;
		}
	}

	ASSERT(found);

	return m_variables[idx].value;
}

Value Interpreter::eval(Block* b)
{
	Value res = true;

	const Opcode& op = b->op();

	switch (op.type()) 
	{
		case Opcode::PRINT:
		{	
			println("%f", eval(b->first_child()).as_number());
		} break;

		case Opcode::REPEAT:
		{	
			println("%f", eval(b->first_child()).as_number());
		} break;

		case Opcode::VALUE:
		{
			res = b->value();
		} break;

		case Opcode::ADD:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = eval(l) + eval(r);
		} break;

		case Opcode::SUBTRACT:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = eval(l) - eval(r);
		} break;

		case Opcode::DIVIDE:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = eval(l) / eval(r);
		} break;

		case Opcode::MULTIPLY:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = eval(l) * eval(r);
		} break;

		case Opcode::EQUALS:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = res && eval(l) == eval(r);
		} break;

		case Opcode::OR:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = res && eval(l) || eval(r);
		} break;

		case Opcode::LESS:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = res && eval(l) < eval(r);
		} break;

		case Opcode::GREATER:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = res && eval(l) > eval(r);
		} break;

		case Opcode::LESS_THAN:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = res && eval(l) <= eval(r);
		} break;

		case Opcode::GREATER_THAN:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_expression() && r->op().is_expression());

			res = res && eval(l) >= eval(r);
		} break;

		case Opcode::NOT:
		{
			Block* l = b->first_child();

			ASSERT(!l->op().is_expression());

			res = res && !eval(l);
		} break;

		case Opcode::SET_VAR:
		{
			Block* l = b->first_child();
			Block* r = l->next();

			ASSERT(l->op().is_string() && r->op().is_expression());

			char identifier[128] = {};

			l->value().as_string(identifier, 128);

			res = set_var(identifier, eval(r));
		} break;

		case Opcode::GET_VAR:
		{
			Block* l = b->first_child();

			l->dump();

			ASSERT(l->op().is_string());

			char identifier[128] = {};

			l->value().as_string(identifier, 128);

			res = get_var(identifier);
		} break;
	}

	return res;
}