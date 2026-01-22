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

	char details[1024];

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

	char details[1024];

	sprintf(details, "Runtime error: %s", what);

	__externref_t msg = js_obj();

	js_set_string(msg, "type", "error");
	js_set_string(msg, "details", details);

	js_post_message(msg);
}

//
// web interface
//

static Arena rt_arena;
static Arena vm_arena;
static Arena compiler_arena;

static atomic_bool should_stop;

atomic_bool* should_stop_ptr()
{
	return &should_stop;
}

void init() 
{
	static uint8_t compiler_mem[4 * 1024 * 1024];
	compiler_arena = arena_make(compiler_mem, sizeof(compiler_mem));

	static uint8_t vm_mem[128 * 1024];
	vm_arena = arena_make(vm_mem, sizeof(vm_mem));

	static uint8_t rt_mem[16 * 1024];
	rt_arena = arena_make(rt_mem, sizeof(rt_mem));

	atomic_store(&should_stop, true);
}

static PQ_Compiler c;
static PQ_VM vm;
static RT_State rt;

void run(__externref_t e)
{
	had_error = false;

	arena_reset(&compiler_arena);
	arena_reset(&vm_arena);
	arena_reset(&rt_arena);

	rt = (RT_State){};

	rt_state_init(&rt_arena, &rt);

	String source = {};

	source.length = (size_t)js_get_int(e, "length");

	if (source.length == 0)
	{
		__externref_t msg = js_obj();

		js_set_string(msg, "type", "error");
		js_set_string(msg, "details", "Empty program");

		js_post_message(msg);

		return;
	}

	source.buffer = arena_push_array(&compiler_arena, char, source.length);
	js_get_string(e, "source", source.buffer);

	// compilation
	c = (PQ_Compiler){};

	pq_compiler_init(&c, &compiler_arena, source, compiler_error_fn);
	rt_declare_procedures(&c);
	
	PQ_CompiledBlob b = pq_compile(&c);

	if (had_error)
	{
		return;
	}

	// execution
	vm = (PQ_VM){};
	
	pq_vm_init(&vm, &vm_arena, &b, vm_error_fn);
	rt_bind_procedures(&vm);

	while (!atomic_load(&should_stop) && pq_execute(&vm)) {}
}

#include <pq/compiler.c>
#include <pq/vm.c>

#include <runtime/canvas.c>
#include <runtime/state.c>