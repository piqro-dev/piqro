#include <pq/vm.h>

#define VERIFY_STACK_OVERFLOW() \
	if (vm->stack_size >= PQ_MAX_STACK_SIZE) \
	{ \
		return TRAP_STACK_OVERFLOW; \
	}

#define VERIFY_STACK_UNDERFLOW() \
	if (vm->stack_size <= 0) \
	{ \
		return TRAP_STACK_UNDERFLOW; \
	}

//
// serialization
//

static void read_from_blob(PQ_VM* vm, const PQ_CompiledBlob* b, void* out, size_t type_size)
{
	__builtin_memcpy(out, b->buffer + vm->bp, type_size);
	vm->bp += type_size;
}

static void read_magic(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	uint32_t magic = 0;

	read_from_blob(vm, b, &magic, sizeof(uint32_t));

	if (magic != __builtin_bswap32('PIQR'))
	{
		vm->error("Invalid magic");
	}
}

static void read_immediates(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	read_from_blob(vm, b, &vm->immediate_count, sizeof(uint8_t));

	vm->immediates = arena_push_array(vm->arena, PQ_Value, vm->immediate_count);

	for (uint8_t i = 0; i < vm->immediate_count; i++)
	{
		PQ_Value v = {};

		read_from_blob(vm, b, &v.type, sizeof(PQ_ValueType));

		switch (v.type)
		{
			case VALUE_UNDEFINED: break;
			
			case VALUE_NUMBER:  read_from_blob(vm, b, &v.n, sizeof(float)); break; 
			case VALUE_BOOLEAN: read_from_blob(vm, b, &v.b, sizeof(bool)); break; 

			case VALUE_STRING:
			{
				uint16_t start = vm->bp;
				uint16_t end = start;

				while (b->buffer[vm->bp++])
				{
					end++;
				}

				v.s = str_copy_c_str_from_to(vm->arena, (char*)b->buffer, start, end);
			} break;

			default: vm->error("Invalid value type");
		}

		vm->immediates[i] = v;
	}
}

static void read_procedures(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	read_from_blob(vm, b, &vm->proc_info_count, sizeof(uint8_t));

	vm->proc_infos = arena_push_array(vm->arena, PQ_ProcedureInfo, vm->proc_info_count);

	for (uint8_t i = 0; i < vm->proc_info_count; i++)
	{
		PQ_ProcedureInfo pi = {};

		read_from_blob(vm, b, &pi.foreign, sizeof(bool));
		read_from_blob(vm, b, &pi.local_count, sizeof(uint8_t));
		read_from_blob(vm, b, &pi.arg_count, sizeof(uint8_t));
		read_from_blob(vm, b, &pi.first_inst, sizeof(uint16_t));
	
		vm->proc_infos[i] = pi;
	}
}

static void read_instructions(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	read_from_blob(vm, b, &vm->instruction_count, sizeof(uint16_t));

	vm->instructions = arena_push_array(vm->arena, PQ_Instruction, vm->instruction_count);

	for (uint16_t i = 0; i < vm->instruction_count; i++)
	{
		PQ_Instruction it = {};

		read_from_blob(vm, b, &it.type, sizeof(PQ_InstructionType));

		if (pq_inst_needs_arg(it.type))
		{
			read_from_blob(vm, b, &it.arg, sizeof(uint16_t));
		}
	
		vm->instructions[i] = it;
	}
}

static void read_blob(PQ_VM* vm, const PQ_CompiledBlob* b)
{	
	read_magic(vm, b);
	read_immediates(vm, b);
	read_procedures(vm, b);
	read_instructions(vm, b);
}

//
// instructions
//

#define VERIFY_STACK_OVERFLOW() \
	if (vm->stack_size >= PQ_MAX_STACK_SIZE) \
	{ \
		return TRAP_STACK_OVERFLOW; \
	}

#define VERIFY_STACK_UNDERFLOW() \
	if (vm->stack_size <= 0) \
	{ \
		return TRAP_STACK_UNDERFLOW; \
	}

static PQ_Trap LOAD_NULL(PQ_VM* vm)
{
	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = pq_value_undefined();

	vm->ip++;

	return TRAP_SUCCESS;
}

static PQ_Trap CALL(PQ_VM* vm, uint16_t idx)
{
	if (idx >= vm->proc_info_count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	if (vm->call_frame_count >= PQ_MAX_CALL_FRAMES)
	{
		return TRAP_STACK_OVERFLOW;
	}
	
	PQ_ProcedureInfo pi = vm->proc_infos[idx];

	PQ_CallFrame* cf = &vm->call_frames[vm->call_frame_count++];

	cf->return_ip = vm->ip + 1;

	cf->arg_count = pi.arg_count;
	cf->local_count = pi.local_count;
	
	cf->stack_base = vm->stack_size;
	cf->local_base = vm->local_count;
	
	// push arguments
	for (uint8_t i = 0; i < cf->arg_count; i++) 
	{
		VERIFY_STACK_UNDERFLOW();
		
		vm->locals[vm->local_count++] = vm->stack[--vm->stack_size];
	}

	if (!pi.foreign)
	{
		// push locals found in procedure
		for (uint8_t i = cf->arg_count; i < cf->local_count; i++) 
		{
			vm->locals[vm->local_count++] = pq_value_undefined();
		}
		
		vm->ip = pi.first_inst;
	}
	else
	{
		// if the procedure is foreign, clean up manually
		if (!pi.proc)
		{
			return TRAP_UNDEFINED_FOREIGN;
		}

		pi.proc(vm);

		VERIFY_STACK_UNDERFLOW();

		if (vm->stack[vm->stack_size - 1].type == VALUE_UNDEFINED)
		{
			--vm->stack_size;
		}

		const PQ_CallFrame cf = vm->call_frames[--vm->call_frame_count];

		vm->local_count = cf.local_base;

		vm->ip++;
	}

	return TRAP_SUCCESS;
}

static PQ_Trap LOAD_IMMEDIATE(PQ_VM* vm, uint16_t idx)
{
	if (idx >= vm->immediate_count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = vm->immediates[idx];

	vm->ip++;

	return TRAP_SUCCESS;
}

static PQ_Trap LOAD_LOCAL(PQ_VM* vm, uint16_t idx)
{
	if (vm->call_frame_count > 0)
	{
		idx = vm->call_frames[vm->call_frame_count - 1].local_base + idx;
	}

	if (idx >= PQ_MAX_LOCALS)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = vm->locals[idx];

	vm->ip++;

	return TRAP_SUCCESS;
}

static PQ_Trap STORE_LOCAL(PQ_VM* vm, uint16_t idx)
{
	if (vm->call_frame_count > 0)
	{
		idx = vm->call_frames[vm->call_frame_count - 1].local_base + idx;
	}

	if (idx >= PQ_MAX_LOCALS)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	vm->locals[idx] = vm->stack[--vm->stack_size];

	vm->ip++;

	return TRAP_SUCCESS;
}

static PQ_Trap JUMP(PQ_VM* vm, uint16_t to)
{
	if (to >= vm->instruction_count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	vm->ip = to;

	return TRAP_SUCCESS;
}

static PQ_Trap JUMP_COND(PQ_VM* vm, uint16_t to)
{
	if (to >= vm->instruction_count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	VERIFY_STACK_UNDERFLOW();

	vm->ip = pq_value_as_boolean(vm->stack[--vm->stack_size]) ? to : vm->ip + 1;

	return TRAP_SUCCESS;
}

// these ones are very repetative
#define DEFINE_OPS \
	OP(ADD, add) \
	OP(SUB, sub) \
	OP(DIV, div) \
	OP(MUL, mul) \
	OP(MOD, mod) \
	OP(AND, and) \
	OP(OR, or) \
	OP(GREATER_THAN, gt) \
	OP(LESS_THAN, lt) \
	OP(EQUALS, equals) \
	OP(GREATER, greater) \
	OP(LESS, less)

#define OP(name, op) \
	static PQ_Trap name(PQ_VM* vm) \
	{ \
		VERIFY_STACK_UNDERFLOW(); \
		\
		PQ_Value r = vm->stack[--vm->stack_size]; \
		\
		VERIFY_STACK_UNDERFLOW(); \
		\
		PQ_Value l = vm->stack[--vm->stack_size]; \
		\
		VERIFY_STACK_OVERFLOW(); \
		\
		vm->stack[vm->stack_size++] = pq_value_##op(l, r); \
		\
		vm->ip++; \
		\
		return TRAP_SUCCESS; \
	}

DEFINE_OPS

static PQ_Trap NOT(PQ_VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	PQ_Value l = vm->stack[--vm->stack_size];

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = pq_value_not(l);

	vm->ip++;

	return TRAP_SUCCESS;
}

static PQ_Trap NEGATE(PQ_VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	PQ_Value l = vm->stack[--vm->stack_size];

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = pq_value_mul(l, pq_value_number(-1.0f));

	vm->ip++;

	return TRAP_SUCCESS;
}

static PQ_Trap RETURN(PQ_VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	if (vm->stack[vm->stack_size - 1].type == VALUE_UNDEFINED)
	{
		--vm->stack_size;
	}

	if (vm->call_frame_count <= 0)
	{
		return TRAP_STACK_UNDERFLOW;
	}

	const PQ_CallFrame cf = vm->call_frames[--vm->call_frame_count];

	vm->stack_size = cf.stack_base;
	vm->local_count = cf.local_base;

	vm->ip = cf.return_ip;

	return TRAP_SUCCESS;
}

static PQ_Trap HALT(PQ_VM* vm)
{
	vm->stack_size = 0;
	vm->local_count = 0;
	vm->call_frame_count = 0;

	return TRAP_HALT_EXECUTION;
}

void pq_vm_init(PQ_VM* vm, Arena* arena, const PQ_CompiledBlob* b, PQ_VMErrorFn error)
{
	if (b->size > PQ_MAX_BLOB_SIZE)
	{
		vm->error("Provided blob is too big");
	}

	vm->arena = arena;

	vm->error = error;

	read_blob(vm, b);

	vm->call_frames = arena_push_array(arena, PQ_CallFrame, PQ_MAX_CALL_FRAMES);
	vm->stack = arena_push_array(arena, PQ_Value, PQ_MAX_STACK_SIZE);
	vm->locals = arena_push_array(arena, PQ_Value, PQ_MAX_LOCALS);
}

PQ_Trap pq_vm_execute(PQ_VM* vm)
{
	PQ_Trap r = TRAP_SUCCESS;

	if (vm->ip >= vm->instruction_count)
	{
		return TRAP_OUT_OF_BOUNDS;
	}

	PQ_Instruction it = vm->instructions[vm->ip];

	switch (it.type)
	{
		case INST_CALL:           r = CALL(vm, it.arg); break;
		case INST_LOAD_NULL:      r = LOAD_NULL(vm);break;
		case INST_LOAD_IMMEDIATE: r = LOAD_IMMEDIATE(vm, it.arg); break;
		case INST_LOAD_LOCAL:     r = LOAD_LOCAL(vm, it.arg); break;
		case INST_STORE_LOCAL:    r = STORE_LOCAL(vm, it.arg); break;
		case INST_JUMP:           r = JUMP(vm, it.arg); break;
		case INST_JUMP_COND:      r = JUMP_COND(vm, it.arg); break;
		case INST_ADD:            r = ADD(vm); break;
		case INST_SUB:            r = SUB(vm); break;
		case INST_DIV:            r = DIV(vm); break;
		case INST_MUL:            r = MUL(vm); break;
		case INST_MOD:            r = MOD(vm); break;
		case INST_OR:             r = OR(vm); break;
		case INST_GREATER_THAN:   r = GREATER_THAN(vm); break;
		case INST_LESS_THAN:      r = LESS_THAN(vm); break;
		case INST_EQUALS:         r = EQUALS(vm); break;
		case INST_GREATER:        r = GREATER(vm); break;
		case INST_LESS:           r = LESS(vm); break;
		case INST_NOT:            r = NOT(vm); break;
		case INST_NEGATE:         r = NEGATE(vm); break;
		case INST_RETURN:         r = RETURN(vm); break;
		case INST_HALT:           r = HALT(vm); break;

		default: r = TRAP_ILLEGAL_INSTRUCTION; break;
	}

	return r;
}

PQ_Value pq_vm_pop(PQ_VM* vm)
{
	return vm->stack[--vm->stack_size];
}

void pq_vm_push(PQ_VM* vm, PQ_Value v)
{
	vm->stack[vm->stack_size++] = v;
}

PQ_Value pq_vm_get_local(PQ_VM* vm, uint8_t index)
{
	if (vm->call_frame_count > 0)
	{
		index = vm->call_frames[vm->call_frame_count - 1].local_base + index;
	}

	return vm->locals[index];
}

void pq_vm_bind_procedure(PQ_VM* vm, uint8_t index, PQ_NativeProcedure proc)
{
	vm->proc_infos[index].proc = proc;
}