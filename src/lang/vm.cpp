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

static inline Trap LOAD_IMMEDIATE(VM* vm, uint16_t idx)
{
	if (idx > vm->immediates.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, vm->immediates[idx]);

	return TRAP_SUCCESS;
}

static inline Trap LOAD_LOCAL(VM* vm, uint16_t idx)
{
	if (idx >= MAX_VARIABLES)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, vm->variables[idx]);

	return TRAP_SUCCESS;
}

static inline Trap STORE_LOCAL(VM* vm, uint16_t idx)
{
	if (idx > MAX_VARIABLES)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	vm->variables[idx] = *pop(&vm->stack);

	return TRAP_SUCCESS;
}

// TODO: Load proc

static inline Trap LOAD_NULL(VM* vm)
{
	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, make_value());

	return TRAP_SUCCESS;
}

// These ones are very repetative

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
	static inline Trap name(VM* vm) \
	{ \
		VERIFY_STACK_UNDERFLOW(); \
		\
		Value* r = pop(&vm->stack); \
		\
		VERIFY_STACK_UNDERFLOW(); \
		\
		Value* l = pop(&vm->stack); \
		\
		VERIFY_STACK_OVERFLOW(); \
		\
		push(&vm->stack, *l op *r); \
		\
		return TRAP_SUCCESS; \
	}

DEFINE_OPS

static inline Trap NOT(VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	Value* l = pop(&vm->stack);

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, !*l);

	return TRAP_SUCCESS;
}

static inline Trap CALL(VM* vm, uint16_t idx)
{
	if (vm->call_frames.count >= MAX_STACK_SIZE)
	{
		return TRAP_STACK_OVERFLOW;
	}

	CallFrame* cf = push(&vm->call_frames);

	cf->return_ic = vm->ic;

	VERIFY_STACK_UNDERFLOW();

	// Top of the stack is the parameter count 
	cf->arg_count = (uint16_t)as_number(pop(&vm->stack));
	cf->stack_base = vm->stack.count;
	cf->local_base = vm->locals.count;

	// Push locals
	for (uint16_t i = 0; i < cf->arg_count; i++)
	{
		VERIFY_STACK_UNDERFLOW();

		push(&vm->locals, *pop(&vm->stack));
	}

	vm->ic = idx - 1;

	return TRAP_SUCCESS;
}

static inline Trap RET(VM* vm)
{
	const CallFrame* cf = pop(&vm->call_frames);

	trim_end(&vm->locals, cf->local_base);
	trim_end(&vm->stack, cf->stack_base);

	const Value* top = end(vm->stack);

	// All procedures are required to load a value on the stack followed by a RET statement at the end.
	// If the value on the stack is (undefined), which is our NULL type, we assume no usable value will be
	// returned.
	if (top->type != VALUE_UNDEFINED)
	{
		push(&vm->stack, *top);
	}

	vm->ic = cf->return_ic;

	return TRAP_SUCCESS;
}

static inline Trap JUMP(VM* vm, uint16_t to)
{
	if (to > vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	vm->ic = to - 1;

	return TRAP_SUCCESS;
}

static inline Trap JUMP_COND(VM* vm, uint16_t to)
{
	if (to > vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	Value* v = pop(&vm->stack);

	if (as_boolean(v))
	{
		vm->ic = to - 1;
	}	

	return TRAP_SUCCESS;
}

//
// Public
//

static inline void init(VM* vm, Arena* arena, const Array <Instruction> instructions, const Array <Value> immediates)
{
	vm->instructions = instructions;
	vm->immediates = immediates;

	vm->locals = make_array<Value>(arena, MAX_VARIABLES);
	vm->stack = make_array<Value>(arena, MAX_STACK_SIZE);
	vm->call_frames = make_array<CallFrame>(arena, MAX_CALL_FRAMES);

	vm->ic = 0;
}

static inline Trap execute(VM* vm)
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

		case INSTRUCTION_AND:
		{
			r = AND(vm);
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

		case INSTRUCTION_RET:
		{
			r = RET(vm);
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

	if (r == TRAP_SUCCESS)
	{
		vm->ic++;
	}

	return r;
}