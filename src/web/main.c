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


void web_init() 
{
	static Arena rt_arena;

	static uint8_t mem[16 * 1024];
	rt_arena = arena_make(mem, sizeof(mem));

	static RT_State rt;
	rt_state_init(&rt_arena, &rt);
}

void web_run(__externref_t e)
{
	had_error = false;

	Arena compiler_arena;
	
	{
		static uint8_t mem[4 * 1024 * 1024];
		compiler_arena = arena_make(mem, sizeof(mem));
	}

	// load the source code
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
	PQ_Compiler c = {};

	{
		pq_compiler_init(&c, &compiler_arena, source, compiler_error_fn);
	
		rt_declare_procedures(&c);
	}
	
	PQ_CompiledBlob b = pq_compile(&c);

	// don't even attempt to continue at this point
	if (had_error) 
	{
		return;
	}

	Arena vm_arena;

	{
		static uint8_t mem[128 * 1024];
		vm_arena = arena_make(mem, sizeof(mem));
	}

	// execution
	PQ_VM vm = {};

	{
		pq_vm_init(&vm, &vm_arena, &b, vm_error_fn);
	
		rt_bind_procedures(&vm);
	
		do {} while (pq_execute(&vm));
	}
}

#include <pq/compiler.c>
#include <pq/vm.c>

#include <runtime/canvas.c>
#include <runtime/state.c>