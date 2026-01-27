#include <pq/vm.h>

#define VM_ERROR(...) \
	do \
	{ \
		char what[2048]; \
		sprintf(what, __VA_ARGS__); \
		vm->halt = true; \
		vm->error(what); \
	} while (0)

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
		VM_ERROR("Invalid magic");
	}
}

static void read_immediates(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	read_from_blob(vm, b, &vm->immediate_count, sizeof(uint16_t));

	vm->immediates = arena_push_array(vm->arena, PQ_Value, vm->immediate_count);

	for (uint16_t i = 0; i < vm->immediate_count; i++)
	{
		PQ_Value v = {};

		read_from_blob(vm, b, &v.type, sizeof(PQ_ValueType));

		switch (v.type)
		{
			case VALUE_NULL: break;
			
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

			default: VM_ERROR("Invalid value type");
		}

		vm->immediates[i] = v;
	}
}

static void read_procedures(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	read_from_blob(vm, b, &vm->proc_info_count, sizeof(uint16_t));

	vm->proc_infos = arena_push_array(vm->arena, PQ_ProcedureInfo, vm->proc_info_count);

	for (uint16_t i = 0; i < vm->proc_info_count; i++)
	{
		PQ_ProcedureInfo pi = {};

		read_from_blob(vm, b, &pi.foreign, sizeof(bool));
		read_from_blob(vm, b, &pi.local_count, sizeof(uint16_t));
		read_from_blob(vm, b, &pi.arg_count, sizeof(uint16_t));
		read_from_blob(vm, b, &pi.first_inst, sizeof(uint16_t));

		if (pi.foreign)
		{
			uint16_t start = vm->bp;
			uint16_t end = start;
	
			while (b->buffer[vm->bp++])
			{
				end++;
			}
			
			pi.foreign_name = str_copy_c_str_from_to(vm->arena, (char*)b->buffer, start, end);
		}
		
		vm->proc_infos[i] = pi;
	}
}

static void read_global_count(PQ_VM* vm, const PQ_CompiledBlob* b)
{
	read_from_blob(vm, b, &vm->global_count, sizeof(uint16_t));

	vm->globals = arena_push_array(vm->arena, PQ_Value, vm->global_count);

	for (uint16_t i = 0; i < vm->global_count; i++)
	{
		vm->globals[i] = pq_value_null();
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
	read_global_count(vm, b);
	read_instructions(vm, b);
}

//
// instructions
//

#define VERIFY_STACK_OVERFLOW() \
	if (vm->stack_size >= PQ_MAX_STACK_SIZE) \
	{ \
		VM_ERROR("Stack overflow"); \
	}

#define VERIFY_STACK_UNDERFLOW() \
	if (vm->stack_size <= 0) \
	{ \
		VM_ERROR("Stack underflow"); \
	}

static uint16_t get_local_idx(PQ_VM* vm, uint16_t idx)
{
	if (vm->call_frame_count > 0)
	{
		const PQ_CallFrame* cf = &vm->call_frames[vm->call_frame_count - 1];

		idx = cf->local_base + idx;
	}

	return idx;
}

static void CALL(PQ_VM* vm, uint16_t idx)
{
	if (idx >= vm->proc_info_count)
	{
		VM_ERROR("Callee index out of bounds");
	}

	const PQ_ProcedureInfo* pi = &vm->proc_infos[idx];

	if (vm->call_frame_count >= PQ_MAX_CALL_FRAMES)
	{
		VM_ERROR("Call frame overflow");
	}

	PQ_CallFrame* cf = &vm->call_frames[vm->call_frame_count++];

	cf->return_ip = vm->ip + 1;
	cf->local_base = vm->local_count;

	cf->scratch = scratch_make(vm->arena);

	if (vm->local_count >= PQ_MAX_LOCALS)
	{
		VM_ERROR("Local overflow");
	}

	vm->local_count += pi->arg_count;

	// for convenience, we pop the arguments off the stack in reversed order
	for (uint16_t i = 0; i < pi->arg_count; i++) 
	{
		VERIFY_STACK_UNDERFLOW();

		vm->locals[(vm->local_count - 1) - i] = vm->stack[--vm->stack_size];
	}

	cf->stack_base = vm->stack_size;
	
	if (!pi->foreign)
	{
		// push locals found in procedure
		for (uint16_t i = 0; i < pi->local_count; i++) 
		{
			if (vm->local_count >= PQ_MAX_LOCALS)
			{
				VM_ERROR("Local overflow");
			}

			vm->locals[vm->local_count++] = pq_value_null();
		}
		
		vm->ip = pi->first_inst;
	}
	// foreign procedures use a different calling "convention".
	//
	// instead of relying on a pregenerated return call, 
	// they call their function pointer, clean up the call frame
	// and move on
	else
	{
		if (!pi->proc)
		{
			VM_ERROR("Undefined foreign procedure '%.*s'", s_fmt(pi->foreign_name));
		}

		pi->proc(vm);

		VERIFY_STACK_UNDERFLOW();

		PQ_Value ret = vm->stack[--vm->stack_size];

		if (vm->call_frame_count <= 0)
		{
			VM_ERROR("Call frame underflow");
		}

		PQ_CallFrame cf = vm->call_frames[--vm->call_frame_count];

		vm->stack_size = cf.stack_base;
		vm->local_count = cf.local_base;

		if (ret.type == VALUE_ARRAY)
		{
			VM_ERROR("Invalid array return");
		}

		if (ret.type != VALUE_NULL)
		{
			VERIFY_STACK_OVERFLOW();

			vm->stack[vm->stack_size++] = ret;
		}
		
		scratch_release(cf.scratch);

		vm->ip++;
	}
}

static void LOAD_IMMEDIATE(PQ_VM* vm, uint16_t idx)
{
	if (idx >= vm->immediate_count)
	{
		VM_ERROR("Immediate index out of bounds");
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = vm->immediates[idx];

	vm->ip++;
}

static void LOAD_LOCAL(PQ_VM* vm, uint16_t idx)
{
	idx = get_local_idx(vm, idx);

	if (idx >= PQ_MAX_LOCALS)
	{
		VM_ERROR("Local index out of bounds");
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = vm->locals[idx];

	vm->ip++;
}

static void STORE_LOCAL(PQ_VM* vm, uint16_t idx)
{
	if (vm->call_frame_count == 0 && vm->local_count <= idx)
	{
		vm->local_count = idx + 1;
	}

	idx = get_local_idx(vm, idx);

	if (idx >= PQ_MAX_LOCALS)
	{
		VM_ERROR("Local index out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	vm->locals[idx] = vm->stack[--vm->stack_size];

	vm->ip++;
}

static void LOAD_GLOBAL(PQ_VM* vm, uint16_t idx)
{
	if (idx >= PQ_MAX_GLOBALS)
	{	
		VM_ERROR("Global index out of bounds");
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = vm->globals[idx];

	vm->ip++;
}

static void STORE_GLOBAL(PQ_VM* vm, uint16_t idx)
{
	if (idx >= PQ_MAX_GLOBALS)
	{
		VM_ERROR("Global index out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	vm->globals[idx] = vm->stack[--vm->stack_size];

	vm->ip++;
}

static void LOAD_LOCAL_SUBSCRIPT(PQ_VM* vm, uint16_t idx)
{
	idx = get_local_idx(vm, idx);

	if (idx >= PQ_MAX_LOCALS)
	{
		VM_ERROR("Local index out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	PQ_Value idx_v = vm->stack[--vm->stack_size];

	if (!pq_value_can_be_number(idx_v))
	{
		VM_ERROR("Invalid array subscript");
	}

	PQ_Value* array = &vm->locals[idx];

	int32_t sub_idx = (int32_t)pq_value_as_number(idx_v);

	if (sub_idx >= array->a.count)
	{
		VM_ERROR("Array index out of bounds");
	}
	else if (sub_idx < 0)
	{
		VM_ERROR("Array index out of bounds");
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = array->a.elements[sub_idx];

	vm->ip++;
}

static void STORE_LOCAL_SUBSCRIPT(PQ_VM* vm, uint16_t idx)
{
	idx = get_local_idx(vm, idx);

	if (idx >= PQ_MAX_LOCALS)
	{
		VM_ERROR("Local index out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	PQ_Value idx_v = vm->stack[--vm->stack_size];

	if (!pq_value_can_be_number(idx_v))
	{
		VM_ERROR("Invalid array subscript");
	}

	VERIFY_STACK_UNDERFLOW();

	PQ_Value v = vm->stack[--vm->stack_size];

	PQ_Value* array = &vm->locals[idx];

	if (array->type != VALUE_ARRAY)
	{
		VM_ERROR("Invalid local type");
	}

	int32_t sub_idx = (int32_t)pq_value_as_number(idx_v);

	if (sub_idx >= array->a.count)
	{
		VM_ERROR("Array index out of bounds");
	}
	else if (sub_idx < 0)
	{
		VM_ERROR("Array index out of bounds");
	}

	array->a.elements[sub_idx] = v;

	vm->ip++;
}

static void LOAD_GLOBAL_SUBSCRIPT(PQ_VM* vm, uint16_t idx)
{
	if (idx >= PQ_MAX_GLOBALS)
	{
		VM_ERROR("Global index out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	PQ_Value idx_v = vm->stack[--vm->stack_size];

	if (!pq_value_can_be_number(idx_v))
	{
		VM_ERROR("Invalid array subscript");
	}

	PQ_Value* array = &vm->globals[idx];

	if (array->type != VALUE_ARRAY)
	{
		VM_ERROR("Invalid global type");
	}

	int32_t sub_idx = (int32_t)pq_value_as_number(idx_v);

	if (sub_idx >= array->a.count)
	{
		VM_ERROR("Array index out of bounds");
	}
	else if (sub_idx < 0)
	{
		VM_ERROR("Array index out of bounds");
	}

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = array->a.elements[sub_idx];

	vm->ip++;
}

static void STORE_GLOBAL_SUBSCRIPT(PQ_VM* vm, uint16_t idx)
{
	if (idx >= PQ_MAX_GLOBALS)
	{
		VM_ERROR("Global index out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	PQ_Value idx_v = vm->stack[--vm->stack_size];

	if (!pq_value_can_be_number(idx_v))
	{
		VM_ERROR("Invalid array subscript");
	}

	VERIFY_STACK_UNDERFLOW();

	PQ_Value v = vm->stack[--vm->stack_size];

	PQ_Value* array = &vm->globals[idx];

	if (array->type != VALUE_ARRAY)
	{
		VM_ERROR("Invalid global type");
	}

	int32_t sub_idx = (int32_t)pq_value_as_number(idx_v);

	if (sub_idx >= array->a.count)
	{
		VM_ERROR("Array index out of bounds");
	}
	else if (sub_idx < 0)
	{
		VM_ERROR("Array index out of bounds");
	}

	array->a.elements[sub_idx] = v;

	vm->ip++;
}

static void JUMP(PQ_VM* vm, uint16_t to)
{
	if (to >= vm->instruction_count)
	{
		VM_ERROR("Instruction pointer out of bounds");
	}

	vm->ip = to;
}

static void JUMP_COND(PQ_VM* vm, uint16_t to)
{
	if (to >= vm->instruction_count)
	{
		VM_ERROR("Jump target out of bounds");
	}

	VERIFY_STACK_UNDERFLOW();

	vm->ip = pq_value_as_boolean(vm->stack[--vm->stack_size]) ? to : vm->ip + 1;
}

static void LOAD_NULL(PQ_VM* vm)
{
	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = pq_value_null();

	vm->ip++;
}

// TODO: arrays are the only values that get allocated dynamically at runtime. 
//       they get cleaned up upon leaving a call frame, however, scopes don't 
//       do this at the moment.
static void LOAD_ARRAY(PQ_VM* vm, uint16_t size)
{
	if (vm->arena->offset + (size * sizeof(PQ_Value)) > vm->arena->capacity)
	{
		VM_ERROR("Out of memory");
	}

	PQ_Value array = pq_value_array(vm->arena, size);
	
	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = array;

	vm->ip++;
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
	OP(LESS, less) \
	OP(BW_OR, bw_or) \
	OP(BW_AND, bw_and) \
	OP(BW_XOR, bw_xor) \
	OP(BW_LEFT_SHIFT, bw_left_shift) \
	OP(BW_RIGHT_SHIFT, bw_right_shift) \

#define OP(name, op) \
	static void name(PQ_VM* vm) \
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
	}

DEFINE_OPS

static void NOT(PQ_VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	PQ_Value l = vm->stack[--vm->stack_size];

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = pq_value_not(l);

	vm->ip++;
}

static void NEGATE(PQ_VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	PQ_Value l = vm->stack[--vm->stack_size];

	VERIFY_STACK_OVERFLOW();

	vm->stack[vm->stack_size++] = pq_value_mul(l, pq_value_number(-1.0f));

	vm->ip++;
}

static void RETURN(PQ_VM* vm)
{
	VERIFY_STACK_UNDERFLOW();

	PQ_Value ret = vm->stack[--vm->stack_size];

	if (vm->call_frame_count <= 0)
	{
		VM_ERROR("Call frame underflow");
	}

	PQ_CallFrame cf = vm->call_frames[--vm->call_frame_count];	

	vm->stack_size = cf.stack_base;
	vm->local_count = cf.local_base;

	if (ret.type == VALUE_ARRAY)
	{
		VM_ERROR("Invalid array return");
	}
	
	if (ret.type != VALUE_NULL)
	{
		VERIFY_STACK_OVERFLOW();

		vm->stack[vm->stack_size++] = ret;
	}

	scratch_release(cf.scratch);

	vm->ip = cf.return_ip;
}

static void HALT(PQ_VM* vm)
{
	vm->stack_size = 0;
	vm->local_count = 0;
	vm->call_frame_count = 0;

	vm->halt = true;
}

void pq_vm_init(PQ_VM* vm, Arena* arena, const PQ_CompiledBlob* b, PQ_VMErrorFn error)
{
	if (b->size > PQ_MAX_BLOB_SIZE)
	{
		VM_ERROR("Provided blob is too big");
	}

	vm->arena = arena;

	vm->error = error;

	vm->call_frames = arena_push_array(arena, PQ_CallFrame, PQ_MAX_CALL_FRAMES);
	vm->stack = arena_push_array(arena, PQ_Value, PQ_MAX_STACK_SIZE);
	vm->locals = arena_push_array(arena, PQ_Value, PQ_MAX_LOCALS);

	read_blob(vm, b);

	vm->halt = false;
}

bool pq_execute(PQ_VM* vm)
{
	if (vm->ip >= vm->instruction_count)
	{
		VM_ERROR("Instruction pointer out of bounds");
	}

	PQ_Instruction it = vm->instructions[vm->ip];

	switch (it.type)
	{
		case INST_CALL:                   CALL(vm, it.arg); break;
		case INST_LOAD_IMMEDIATE:         LOAD_IMMEDIATE(vm, it.arg); break;
		case INST_LOAD_LOCAL:             LOAD_LOCAL(vm, it.arg); break;
		case INST_STORE_LOCAL:            STORE_LOCAL(vm, it.arg); break;
		case INST_LOAD_GLOBAL:            LOAD_GLOBAL(vm, it.arg); break;
		case INST_STORE_GLOBAL:           STORE_GLOBAL(vm, it.arg); break;
		case INST_LOAD_LOCAL_SUBSCRIPT:   LOAD_LOCAL_SUBSCRIPT(vm, it.arg); break;
		case INST_STORE_LOCAL_SUBSCRIPT:  STORE_LOCAL_SUBSCRIPT(vm, it.arg); break;
		case INST_LOAD_GLOBAL_SUBSCRIPT:  LOAD_GLOBAL_SUBSCRIPT(vm, it.arg); break;
		case INST_STORE_GLOBAL_SUBSCRIPT: STORE_GLOBAL_SUBSCRIPT(vm, it.arg); break;
		case INST_LOAD_ARRAY:             LOAD_ARRAY(vm, it.arg); break;
		case INST_JUMP:                   JUMP(vm, it.arg); break;
		case INST_JUMP_COND:              JUMP_COND(vm, it.arg); break;
		case INST_LOAD_NULL:              LOAD_NULL(vm); break;
		case INST_ADD:                    ADD(vm); break;
		case INST_SUB:                    SUB(vm); break;
		case INST_DIV:                    DIV(vm); break;
		case INST_MUL:                    MUL(vm); break;
		case INST_MOD:                    MOD(vm); break;
		case INST_AND:                    AND(vm); break;
		case INST_OR:                     OR(vm); break;
		case INST_GREATER_THAN:           GREATER_THAN(vm); break;
		case INST_LESS_THAN:              LESS_THAN(vm); break;
		case INST_EQUALS:                 EQUALS(vm); break;
		case INST_GREATER:                GREATER(vm); break;
		case INST_LESS:                   LESS(vm); break;
		case INST_NOT:                    NOT(vm); break;
		case INST_NEGATE:                 NEGATE(vm); break;
		case INST_BW_OR:                  BW_OR(vm); break;
		case INST_BW_AND:                 BW_AND(vm); break;
		case INST_BW_XOR:                 BW_XOR(vm); break;
		case INST_BW_LEFT_SHIFT:          BW_LEFT_SHIFT(vm); break;
		case INST_BW_RIGHT_SHIFT:         BW_RIGHT_SHIFT(vm); break;
		case INST_RETURN:                 RETURN(vm); break;
		case INST_HALT:                   HALT(vm); break;

		default: VM_ERROR("Illegal instruction %d", it.type);
	}

	return !vm->halt;
}

PQ_Value pq_vm_get_local(PQ_VM* vm, uint16_t idx)
{
	return vm->locals[get_local_idx(vm, idx)];
}

PQ_Value pq_vm_pop(PQ_VM* vm)
{
	return vm->stack[--vm->stack_size];
}

void pq_vm_push(PQ_VM* vm, PQ_Value v)
{
	vm->stack[vm->stack_size++] = v;
}

void pq_vm_bind_foreign_proc(PQ_VM* vm, String name, PQ_NativeProcedure proc)
{
	for (uint16_t i = 0; i < vm->proc_info_count; i++)
	{
		PQ_ProcedureInfo* pi = &vm->proc_infos[i];

		if (str_equals(name, pi->foreign_name))
		{
			pi->proc = proc;
		}
	}
}