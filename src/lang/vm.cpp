#include <lang/vm.h>

#define VERIFY_STACK_OVERFLOW() \
	if (vm->stack.count >= MAX_STACK_SIZE) \
	{ \
		return TRAP_STACK_OVERFLOW; \
	}

#define VERIFY_STACK_UNDERFLOW() \
	if (vm->stack.count <= 0) \
	{ \
		return TRAP_STACK_UNDERFLOW; \
	}

inline Trap LOAD_IMMEDIATE(VM* vm, uint16_t idx)
{
	if (idx >= vm->immediates.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, vm->immediates[idx]);

	vm->ic++;

	return TRAP_SUCCESS;
}

inline Trap LOAD_LOCAL(VM* vm, uint16_t idx)
{
	idx = peek(&vm->call_frames)->local_base + idx;

	if (idx >= MAX_LOCALS)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, vm->locals[idx]);

	vm->ic++;

	return TRAP_SUCCESS;
}

inline Trap STORE_LOCAL(VM* vm, uint16_t idx)
{
	idx = peek(&vm->call_frames)->local_base + idx;

	if (idx >= MAX_LOCALS)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	Value v = *pop(&vm->stack);

	vm->locals[idx] = v;

	vm->ic++;

	return TRAP_SUCCESS;
}

// TODO: Load proc

inline Trap LOAD_NULL(VM* vm)
{
	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, make_value());

	vm->ic++;

	return TRAP_SUCCESS;
}

// These ones are very repetative

#define DEFINE_OPS \
	OP(ADD, +) \
	OP(SUB, -) \
	OP(DIV, /) \
	OP(MUL, *) \
	OP(MOD, %) \
	OP(AND, &&) \
	OP(OR, ||) \
	OP(GREATER_THAN, >=) \
	OP(LESS_THAN, <=) \
	OP(EQUALS, ==) \
	OP(GREATER, >) \
	OP(LESS, <)

#define OP(name, op) \
	inline Trap name(VM* vm) \
	{ \
		VERIFY_STACK_UNDERFLOW(); \
		\
		Value r = *pop(&vm->stack); \
		\
		VERIFY_STACK_UNDERFLOW(); \
		\
		Value l = *pop(&vm->stack); \
		\
		VERIFY_STACK_OVERFLOW(); \
		\
		push(&vm->stack, l op r); \
		\
		vm->ic++; \
		\
		return TRAP_SUCCESS; \
	}

DEFINE_OPS

inline Trap NOT(VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	Value l = *pop(&vm->stack);

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, !l);

	vm->ic++;

	return TRAP_SUCCESS;
}

inline Trap CALL(VM* vm, uint16_t to)
{
	if (to >= vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	if (vm->call_frames.count >= MAX_CALL_FRAMES)
	{
		return TRAP_STACK_OVERFLOW;
	}

	CallFrame* cf = push(&vm->call_frames);

	cf->return_ic = vm->ic + 1;

	VERIFY_STACK_UNDERFLOW();

	// Argument count on top
	cf->arg_count = (uint8_t)as_number(*pop(&vm->stack));
	
	VERIFY_STACK_UNDERFLOW();

	// Local count comes after
	cf->local_count = (uint16_t)as_number(*pop(&vm->stack));
	
	// Save bases
	cf->stack_base = vm->stack.count;
	cf->local_base = vm->locals.count;

	// Push arguments
	for (uint8_t i = 0; i < cf->arg_count; i++) 
	{
		VERIFY_STACK_UNDERFLOW();
		
		push(&vm->locals, *pop(&vm->stack));
	}

	// Push locals
	for (uint8_t i = cf->arg_count; i < cf->local_count; i++) 
	{
		push(&vm->locals, make_value());
	}

	vm->ic = to;

	return TRAP_SUCCESS;
}

inline Trap RETURN(VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	if (peek(&vm->stack)->type == VALUE_UNDEFINED)
	{
		pop(&vm->stack);
	}

	if (vm->call_frames.count <= 0)
	{
		return TRAP_STACK_UNDERFLOW;
	}

	const CallFrame cf = *pop(&vm->call_frames);

	trim_end(&vm->stack, cf.stack_base);
	trim_end(&vm->locals, cf.local_base);

	vm->ic = cf.return_ic;

	return TRAP_SUCCESS;
}

inline Trap JUMP(VM* vm, uint16_t to)
{
	if (to >= vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	vm->ic = to;

	return TRAP_SUCCESS;
}

inline Trap JUMP_COND(VM* vm, uint16_t to)
{
	if (to >= vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	vm->ic = as_boolean(*pop(&vm->stack)) ? to : vm->ic + 1;

	return TRAP_SUCCESS;
}

//
// Public
//

void init(VM* vm, Arena* arena, const Array <Instruction> instructions, const Array <Value> immediates)
{
	vm->instructions = instructions;
	vm->immediates = immediates;

	vm->stack = make_array<Value>(arena, MAX_STACK_SIZE);
	vm->locals = make_array<Value>(arena, MAX_LOCALS);
	vm->call_frames = make_array<CallFrame>(arena, MAX_CALL_FRAMES);

	vm->ic = 0;
}

Trap execute(VM* vm)
{
	Trap r = TRAP_SUCCESS;

	Instruction i = vm->instructions[vm->ic];

	switch (i.type) 
	{
		case INSTRUCTION_LOAD_IMMEDIATE:
		{
			r = LOAD_IMMEDIATE(vm, i.arg);
		} break;
	
		case INSTRUCTION_LOAD_LOCAL:
		{
			r = LOAD_LOCAL(vm, i.arg);	
		} break;
	
		case INSTRUCTION_STORE_LOCAL:
		{
			r = STORE_LOCAL(vm, i.arg);	
		} break;
	
		case INSTRUCTION_LOAD_NULL:
		{
			r = LOAD_NULL(vm);	
		} break;

		case INSTRUCTION_ADD:
		{
			r = ADD(vm);
		} break;

		case INSTRUCTION_SUB:
		{
			r = SUB(vm);
		} break;

		case INSTRUCTION_DIV:
		{
			r = DIV(vm);
		} break;

		case INSTRUCTION_MUL:
		{
			r = MUL(vm);
		} break;

		case INSTRUCTION_MOD:
		{
			r = MOD(vm);
		} break;

		case INSTRUCTION_OR:
		{
			r = OR(vm);
		} break;

		case INSTRUCTION_GREATER_THAN:
		{
			r = GREATER_THAN(vm);
		} break;

		case INSTRUCTION_LESS_THAN:
		{
			r = LESS_THAN(vm);
		} break;

		case INSTRUCTION_EQUALS:
		{
			r = EQUALS(vm);
		} break;

		case INSTRUCTION_GREATER:
		{
			r = GREATER(vm);
		} break;

		case INSTRUCTION_LESS:
		{
			r = LESS(vm);
		} break;

		case INSTRUCTION_NOT:
		{
			r = NOT(vm);
		} break;

		case INSTRUCTION_CALL:
		{
			r = CALL(vm, i.arg);
		} break;

		case INSTRUCTION_RETURN:
		{
			r = RETURN(vm);
		} break;

		case INSTRUCTION_JUMP:
		{
			r = JUMP(vm, i.arg);
		} break;

		case INSTRUCTION_JUMP_COND:
		{
			r = JUMP_COND(vm, i.arg);
		} break;

		case INSTRUCTION_NOOP:
		{
			// ... Do nothing
			vm->ic++;
		} break;

		case INSTRUCTION_HALT:
		{
			r = TRAP_HALT_EXECUTION;
		} break;

		default:
		{
			r = TRAP_ILLEGAL_INSTRUCTION;
		} break;
	}

	return r;
}