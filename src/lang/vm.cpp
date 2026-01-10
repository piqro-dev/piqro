#include <lang/vm.h>

template <typename T>
inline T read(VM* vm)
{
	T v = {};

	__builtin_memcpy(&v, vm->blob + vm->bp, sizeof(T));
	vm->bp += sizeof(T);

	return v;
}

inline void read_magic(VM* vm)
{
	uint32_t magic = read<uint32_t>(vm);

	if (magic != __builtin_bswap32('PIQR'))
	{
		errorln("Error: Invalid magic");
	}
}

inline void read_immediates(VM* vm, Arena* arena)
{
	uint8_t count = read<uint8_t>(vm);

	vm->immediates = make_array<Value>(arena, count);

	for (uint8_t i = 0; i < count; i++)
	{
		Value v = {};

		v.type = read<ValueType>(vm);

		switch (v.type)
		{
			case VALUE_UNDEFINED: break;
			
			case VALUE_NUMBER:  v.number  = read<float>(vm); break; 
			case VALUE_BOOLEAN: v.boolean = read<bool>(vm); break; 

			case VALUE_STRING:
			{
				uint8_t len = 0;

				while (v.string[len])
				{
					v.string[len++] = read<char>(vm);
				}
			} break;

			default: { errorln("Error: Invalid value type"); };
		}

		push(&vm->immediates, v);
	}
}

inline void read_procedures(VM* vm, Arena* arena)
{
	uint8_t count = read<uint8_t>(vm);

	vm->procedure_infos = make_array<ProcedureInfo>(arena, count);

	for (uint8_t i = 0; i < vm->procedure_infos.capacity; i++)
	{
		ProcedureInfo pi = {};

		pi.foreign = read<bool>(vm);
		pi.local_count = read<uint8_t>(vm);
		pi.arg_count = read<uint8_t>(vm);
		pi.first_inst = read<uint16_t>(vm);
	
		push(&vm->procedure_infos, pi);
	}
}

inline void read_instructions(VM* vm, Arena* arena)
{
	uint16_t count = read<uint16_t>(vm);

	vm->instructions = make_array<Instruction>(arena, count);

	for (uint16_t i = 0; i < vm->instructions.capacity; i++)
	{
		Instruction it = {};

		it.type = read<InstructionType>(vm);

		if (needs_argument(it.type))
		{
			it.arg = read<uint16_t>(vm);
		}

		push(&vm->instructions, it);
	}
}

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

inline Trap LOAD_NULL(VM* vm)
{
	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, make_value());

	vm->ip++;

	return TRAP_SUCCESS;
}

inline Trap CALL(VM* vm, uint16_t idx)
{
	if (idx >= vm->procedure_infos.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	if (vm->call_frames.count >= MAX_CALL_FRAMES)
	{
		return TRAP_STACK_OVERFLOW;
	}
	
	ProcedureInfo pi = vm->procedure_infos[idx];

	CallFrame* cf = push(&vm->call_frames);

	cf->return_ip = vm->ip + 1;

	cf->arg_count = pi.arg_count;
	cf->local_count = pi.local_count;
	
	cf->stack_base = vm->stack.count;
	cf->local_base = vm->locals.count;

	// push arguments
	for (uint8_t i = 0; i < cf->arg_count; i++) 
	{
		VERIFY_STACK_UNDERFLOW();
		
		push(&vm->locals, *pop(&vm->stack));
	}

	// push locals
	for (uint8_t i = cf->arg_count; i < cf->local_count; i++) 
	{
		push(&vm->locals, make_value());
	}

	vm->ip = pi.first_inst;

	return TRAP_SUCCESS;
}

inline Trap LOAD_IMMEDIATE(VM* vm, uint16_t idx)
{
	if (idx >= vm->immediates.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, vm->immediates[idx]);

	vm->ip++;

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

	vm->ip++;

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

	vm->ip++;

	return TRAP_SUCCESS;
}

inline Trap JUMP(VM* vm, uint16_t to)
{
	if (to >= vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	vm->ip = to;

	return TRAP_SUCCESS;
}

inline Trap JUMP_COND(VM* vm, uint16_t to)
{
	if (to >= vm->instructions.count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	vm->ip = as_boolean(*pop(&vm->stack)) ? to : vm->ip + 1;

	return TRAP_SUCCESS;
}

// these ones are very repetative
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
		vm->ip++; \
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

	vm->ip++;

	return TRAP_SUCCESS;
}

inline Trap NEGATE(VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	Value l = *pop(&vm->stack);

	VERIFY_STACK_OVERFLOW();

	push(&vm->stack, l * make_value(-1.0f));

	vm->ip++;

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

	vm->ip = cf.return_ip;

	return TRAP_SUCCESS;
}

void init(VM* vm, Arena* arena, uint8_t* blob, uint16_t blob_size)
{
	vm->ip = 0;
	vm->bp = 0;

	vm->blob = blob;
	vm->blob_size = blob_size;

	if (vm->blob_size > MAX_BLOB_SIZE)
	{
		errorln("Error: Provided blob is too big.");
	}

	vm->call_frames = make_array<CallFrame>(arena, MAX_CALL_FRAMES);
	vm->stack = make_array<Value>(arena, MAX_STACK_SIZE);
	vm->locals = make_array<Value>(arena, MAX_LOCALS);

	read_magic(vm);

	read_immediates(vm, arena);
	read_procedures(vm, arena);
	read_instructions(vm, arena);
}

Trap execute(VM* vm)
{
	Trap r = TRAP_SUCCESS;

	Instruction it = vm->instructions[vm->ip];

	switch (it.type)
	{
		case INSTRUCTION_CALL:           r = CALL(vm, it.arg); break;
		case INSTRUCTION_LOAD_NULL:      r = LOAD_NULL(vm);break;
		case INSTRUCTION_LOAD_IMMEDIATE: r = LOAD_IMMEDIATE(vm, it.arg); break;
		case INSTRUCTION_LOAD_LOCAL:     r = LOAD_LOCAL(vm, it.arg); break;
		case INSTRUCTION_STORE_LOCAL:    r = STORE_LOCAL(vm, it.arg); break;
		case INSTRUCTION_JUMP:           r = JUMP(vm, it.arg); break;
		case INSTRUCTION_JUMP_COND:      r = JUMP_COND(vm, it.arg); break;
		case INSTRUCTION_ADD:            r = ADD(vm); break;
		case INSTRUCTION_SUB:            r = SUB(vm); break;
		case INSTRUCTION_DIV:            r = DIV(vm); break;
		case INSTRUCTION_MUL:            r = MUL(vm); break;
		case INSTRUCTION_MOD:            r = MOD(vm); break;
		case INSTRUCTION_OR:             r = OR(vm); break;
		case INSTRUCTION_GREATER_THAN:   r = GREATER_THAN(vm); break;
		case INSTRUCTION_LESS_THAN:      r = LESS_THAN(vm); break;
		case INSTRUCTION_EQUALS:         r = EQUALS(vm); break;
		case INSTRUCTION_GREATER:        r = GREATER(vm); break;
		case INSTRUCTION_LESS:           r = LESS(vm); break;
		case INSTRUCTION_NOT:            r = NOT(vm); break;
		case INSTRUCTION_NEGATE:         r = NEGATE(vm); break;
		case INSTRUCTION_RETURN:         r = RETURN(vm); break;
		case INSTRUCTION_HALT:           r = TRAP_HALT_EXECUTION; break;

		default:                         r = TRAP_ILLEGAL_INSTRUCTION; break;
	}

	return r;
}