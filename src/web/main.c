#include <web/support.h>

#include <base/log.h>

#include <pq/compiler.h>

#include <pq/vm.h>

static bool got_error;

void compiler_error_fn(uint16_t line, const char* what)
{
	char msg[2048];
	sprintf(msg, "Compilation error: line %d: %s", line, what);

	js_alert(msg);

	got_error = true;
}

void vm_error_fn(const char* what)
{
	char msg[2048];
	sprintf(msg, "Runtime error: %s", what);

	js_alert(msg);

	got_error = true;
}

void dump_stack(PQ_VM* vm)
{
	println("Stack:");

	if (vm->stack_size > 0)
	{
		for (uint8_t i = 0; i < vm->stack_size; i++)
		{
			String v = pq_value_as_string(vm->arena, vm->stack[i]);

			println("   [%d] %.*s", i, s_fmt(v));
		}
	}
	else
	{
		println("   (empty)");
	}
}

void dump_all_instructions(PQ_Compiler* c, PQ_VM* vm)
{
	for (uint16_t i = 0; i < vm->instruction_count; i++)
	{
		PQ_Instruction it = vm->instructions[i];

		if (pq_inst_needs_arg(it.type))
		{
			if (it.type == INST_CALL)
			{
				println("  %-4d | %-20s %d (%.*s)", i, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->procedures[it.arg].name));
			}
			else
			{
				println("  %-4d | %-20s %d", i, pq_inst_to_c_str(it.type), it.arg);
			}
		}
		else
		{
			println("  %-4d | %-20s", i, pq_inst_to_c_str(it.type));
		}
	}
}

void dump_instruction(PQ_Compiler* c, PQ_VM* vm)
{
	PQ_Instruction it = vm->instructions[vm->ip];

	if (pq_inst_needs_arg(it.type))
	{
		if (it.type == INST_CALL)
		{
			println("\n-> %d | %-20s %d (%.*s)", vm->ip, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->procedures[it.arg].name));
		}
		else
		{
			println("\n-> %d | %-20s %d", vm->ip, pq_inst_to_c_str(it.type), it.arg);
		}
	}
	else
	{
		println("\n-> %d | %-20s", vm->ip, pq_inst_to_c_str(it.type));
	}
}

static uint8_t mem[4 * 1024 * 1024];

static uint32_t ctx_ref;

// clear_background(r, g, b)
void clear_background_proc(PQ_VM* vm)
{
	const uint8_t r = (uint8_t)pq_value_as_number(pq_get_local(vm, 0));
	const uint8_t g = (uint8_t)pq_value_as_number(pq_get_local(vm, 1));
	const uint8_t b = (uint8_t)pq_value_as_number(pq_get_local(vm, 2));

	char fill_style[64] = {};
	sprintf(fill_style, "rgb(%d, %d, %d)", r, g, b);

	js_set_string(load_ref(ctx_ref), "fillStyle", fill_style);

	const int64_t w = js_get_int(js_get_element_by_id("main-canvas"), "width");
	const int64_t h = js_get_int(js_get_element_by_id("main-canvas"), "height");

	js_clear_rect(load_ref(ctx_ref), 0, 0, w, h);
	js_fill_rect(load_ref(ctx_ref), 0, 0, w, h);

	pq_return(vm);
}

void sin_proc(PQ_VM* vm)
{
	pq_return_value(vm, pq_value_number(__builtin_sinf(pq_value_as_number(pq_get_local(vm, 0)))));
}

static Arena arena;

void run_button_onclick(__externref_t e)
{
	got_error = false;

	arena_reset(&arena);

	String source = {};
	
	{
		__externref_t editor = js_get_element_by_id("editor");

		source.length = js_get_int(js_get(editor, "value"), "length");
		source.buffer = arena_push_array(&arena, char, source.length + 1);

		js_get_string(editor, "value", source.buffer);
	}

	PQ_Compiler c = {};

	{
		pq_init_compiler(&c, &arena, source, compiler_error_fn);
	
		pq_declare_foreign_proc(&c, s("clear_background"), 3);
		pq_declare_foreign_proc(&c, s("sin"), 1);
	}

	PQ_CompiledBlob b = pq_compile(&c);

	PQ_VM vm = {};
	
	{
		pq_init_vm(&vm, &arena, &b, vm_error_fn);
	
		pq_bind_foreign_proc(&vm, s("clear_background"), clear_background_proc);
		pq_bind_foreign_proc(&vm, s("sin"), sin_proc);
	}

	if (!got_error)
	{
		bool r = true;
	
		while (r)
		{
			r = pq_execute(&vm); 
		}
	}
}

int main()
{
	arena = arena_make(mem, sizeof(mem));

	ctx_ref = push_ref(js_get_context(js_get_element_by_id("main-canvas"), "2d"));

	js_add_event_listener(js_get_element_by_id("run-button"), "click", run_button_onclick);
}

#include <pq/compiler.c>

#include <pq/vm.c>