#include <stdatomic.h>

#include <web/support.h>

#include <pq/compiler.h>
#include <pq/vm.h>

#include <runtime/canvas.h>
#include <runtime/state.h>

static bool had_error = false;

static void compiler_error_fn(uint16_t line, const char* what)
{
	if (had_error)
	{
		return;
	}

	had_error = true;

	char details[1024] = {};

	sprintf(details, "Compilation error: line %d: %s", line, what);

	__externref_t msg = js_obj();

	js_set_string(msg, "type", "error");
	js_set_string(msg, "details", details);

	js_post_message(msg);
}

static void vm_error_fn(const char* what)
{
	if (had_error)
	{
		return;
	}

	had_error = true;

	char details[1024] = {};

	sprintf(details, "Runtime error: %s", what);

	__externref_t msg = js_obj();

	js_set_string(msg, "type", "error");
	js_set_string(msg, "details", details);

	js_post_message(msg);
}

static void dump_procedures(PQ_Compiler* c)
{
	printf("Procedures:\n");

	for (uint16_t i = 0; i < c->procedure_count; i++)
	{
		PQ_Procedure p = c->procedures[i];

		printf("  name: %.*s\n", s_fmt(p.name));
		printf("  idx: %d\n", p.idx);
		printf("  local_count: %d\n", p.local_count);
		printf("  arg_count: %d\n", p.arg_count);
		printf("  first_inst: %d\n", p.scope.first_inst);
		printf("  foreign: %s\n", p.foreign ? "true" : "false");
	
		printf("\n");
	}
}

static void dump_instructions(PQ_Compiler* c)
{
	Scratch scratch = scratch_make(c->arena);

	for (uint16_t i = 0; i < c->instruction_count; i++)
	{
		PQ_Instruction it = c->instructions[i];

		if (pq_inst_needs_arg(it.type))
		{
			if (it.type == INST_CALL)
			{
				printf("  %-4d | %-25s %d (%.*s)\n", i, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->procedures[it.arg].name));
			}
			else if (it.type == INST_LOAD_IMMEDIATE)
			{
				printf("  %-4d | %-25s %d (%.*s)\n", i, pq_inst_to_c_str(it.type), it.arg, s_fmt(pq_value_as_string(scratch.arena, c->immediates[it.arg])));
			}
			else
			{
				printf("  %-4d | %-25s %d\n", i, pq_inst_to_c_str(it.type), it.arg);
			}
		}
		else
		{
			printf("  %-4d | %-25s\n", i, pq_inst_to_c_str(it.type));
		}
	}

	scratch_release(scratch);
}

void dump_stack(PQ_VM* vm)
{
	printf("Stack:\n");

	if (vm->stack_size > 0)
	{
		Scratch scratch = scratch_make(vm->arena);

		for (uint8_t i = 0; i < vm->stack_size; i++)
		{
			String v = pq_value_as_string(scratch.arena, vm->stack[i]);

			printf("   [%d] %.*s, %s\n", i, s_fmt(v), pq_value_to_c_str(vm->stack[i].type));
		}

		scratch_release(scratch);
	}
	else
	{
		printf("   (empty)\n");
	}
}

static void post_rt(const RT_State* rt)
{
	__externref_t msg = js_obj();

	js_set_string(msg, "type", "state");
	js_set_int(msg, "frameBuffer", (intptr_t)rt->canvas.frame_buffer);

	js_set_int(msg, "leftKeyPtr", (intptr_t)&rt->left_key);
	js_set_int(msg, "rightKeyPtr", (intptr_t)&rt->right_key);
	js_set_int(msg, "upKeyPtr", (intptr_t)&rt->up_key);
	js_set_int(msg, "downKeyPtr", (intptr_t)&rt->down_key);

	js_set_int(msg, "aKeyPtr", (intptr_t)&rt->a_key);
	js_set_int(msg, "bKeyPtr", (intptr_t)&rt->b_key);

	js_set_int(msg, "timeSinceStartPtr", (intptr_t)&rt->time_since_start);

	js_post_message(msg);
}

//
// web interface
//

static Arena compiler_arena;
static Arena rt_arena;

static atomic_bool should_stop;

atomic_bool* should_stop_ptr()
{
	return &should_stop;
}

static PQ_CompiledBlob blob;

void init() 
{
	static uint8_t compiler_mem[4 * 1024 * 1024];
	compiler_arena = arena_make(compiler_mem, sizeof(compiler_mem));

	static uint8_t rt_mem[256 * 1024];
	rt_arena = arena_make(rt_mem, sizeof(rt_mem));

	atomic_store(&should_stop, true);
}

void compile_and_export(__externref_t e)
{
	had_error = false;

	arena_reset(&compiler_arena);
	arena_reset(&rt_arena);

	RT_State rt = {};

	rt_state_init(&rt_arena, &rt);

	String source = {};

	source.length = (size_t)js_get_int(e, "length");
	source.buffer = arena_push_array(&compiler_arena, char, source.length);
	
	js_get_string(e, "source", source.buffer);

	PQ_Compiler c = {};

	pq_compiler_init(&c, &compiler_arena, source, compiler_error_fn);
	rt_declare_procedures(&c);
	
	blob = pq_compile(&c);

	if (had_error)
	{
		return;
	}

	{
		__externref_t msg = js_obj();

		js_set_string(msg, "type", "output");
		js_set_int(msg, "buffer", (intptr_t)blob.buffer);
		js_set_int(msg, "size", blob.size);

		js_post_message(msg);
	}
}

void run_from_blob(__externref_t e)
{
	arena_reset(&rt_arena);

	RT_State rt = {};

	rt_state_init(&rt_arena, &rt);

	post_rt(&rt);

	blob = (PQ_CompiledBlob){};

	blob.size = js_get_int(e, "length");
	blob.buffer = arena_push_array(&rt_arena, uint8_t, blob.size);

	js_memcpy(blob.buffer, js_get(e, "buffer"), blob.size);

	PQ_VM vm = {};

	pq_vm_init(&vm, &rt_arena, &blob, vm_error_fn);
	rt_bind_procedures(&vm);

	while (!atomic_load(&should_stop) && pq_execute(&vm)) {}
}

void compile_and_run(__externref_t e)
{
	had_error = false;

	arena_reset(&compiler_arena);
	arena_reset(&rt_arena);

	RT_State rt = {};

	rt_state_init(&rt_arena, &rt);

	String source = {};

	source.length = (size_t)js_get_int(e, "length");
	source.buffer = arena_push_array(&compiler_arena, char, source.length);
	
	js_get_string(e, "source", source.buffer);

	PQ_Compiler c = {};

	pq_compiler_init(&c, &compiler_arena, source, compiler_error_fn);
	rt_declare_procedures(&c);
	
	blob = pq_compile(&c);

	if (had_error)
	{
		return;
	}

	post_rt(&rt);

	printf("blob size: %d", blob.size);

	PQ_VM vm = {};

	pq_vm_init(&vm, &rt_arena, &blob, vm_error_fn);
	rt_bind_procedures(&vm);

	while (!atomic_load(&should_stop) && pq_execute(&vm)) {}
}

#include <pq/compiler.c>
#include <pq/vm.c>

#include <runtime/canvas.c>
#include <runtime/state.c>