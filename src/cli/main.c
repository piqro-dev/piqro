#include <base/log.h>

#include <pq/compiler.h>

#include <pq/vm.h>

void compiler_error_fn(uint16_t line, const char* what)
{
	errorln("Compilation error: line %d: %s", line, what);
	exit(1);
}

void vm_error_fn(const char* what)
{
	errorln("Runtime error: %s", what);
	exit(1);
}

void print_proc(PQ_VM* vm)
{
	String v = pq_value_as_string(vm->arena, pq_get_local(vm, 0));

	println("%.*s", s_fmt(v));

	pq_return(vm);
}

void sin_proc(PQ_VM* vm)
{
	float v = pq_value_as_number(pq_get_local(vm, 0));

	pq_return_value(vm, pq_value_number(__builtin_sinf(v)));
}

void test_proc(PQ_VM* vm)
{
	bool expr = pq_value_as_boolean(pq_get_local(vm, 0));
	String name = pq_value_as_string(vm->arena, pq_get_local(vm, 1));

	if (expr)
	{
		println("TEST: [SUCCESS] %.*s", s_fmt(name));
	}
	else
	{
		println("TEST: [FAILURE] %.*s", s_fmt(name));
	}

	pq_return(vm);
}

void dump_stack(PQ_VM* vm)
{
	println("Stack:");

	if (vm->stack_size > 0)
	{
		for (uint8_t i = 0; i < vm->stack_size; i++)
		{
			String v = pq_value_as_string(vm->arena, vm->stack[i]);

			println("   [%d] %.*s, %s", i, s_fmt(v), pq_value_to_c_str(vm->stack[i].type));
		}
	}
	else
	{
		println("   (empty)");
	}
}

void dump_procedures(PQ_Compiler* c)
{
	println("Procedures:");

	for (uint16_t i = 0; i < c->procedure_count; i++)
	{
		PQ_Procedure p = c->procedures[i];

		println("  name: %.*s", s_fmt(p.name));
		println("  idx: %d", p.idx);
		println("  local_count: %d", p.local_count);
		println("  arg_count: %d", p.arg_count);
		println("  foreign: %s", p.foreign ? "true" : "false");
	
		println("");
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
				println("  %-4d | %-25s %d (%.*s)", i, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->procedures[it.arg].name));
			}
			else
			{
				println("  %-4d | %-25s %d", i, pq_inst_to_c_str(it.type), it.arg);
			}
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
			println("\n-> %d | %-25s %d (%.*s)", vm->ip, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->procedures[it.arg].name));
		}
		else
		{
			println("\n-> %d | %-25s %d", vm->ip, pq_inst_to_c_str(it.type), it.arg);
		}
	}
	else
	{
		println("\n-> %d | %-25s", vm->ip, pq_inst_to_c_str(it.type));
	}
}

void dump_global_count(PQ_Compiler* c)
{
	uint16_t count = 0;

	for (uint16_t i = 0; i < c->variable_count; i++)
	{
		count++;
	}

	println("global count: %d", count);
	println("");
}

void dump_state(PQ_VM* vm)
{	
	println("\nVM info:");
	println("local count: %d", vm->local_count);
	println("stack size:  %d", vm->stack_size);
	
	println("\nVM locals:");

	for (uint16_t i = 0; i < vm->local_count; i++)
	{
		String v = pq_value_as_string(vm->arena, vm->locals[i]);
		println("   [%d] %.*s, %s", i, s_fmt(v), pq_value_to_c_str(vm->locals[i].type));
	}
}

static constexpr const char source[] = 
{
	#embed "test_bed.pq" 
	,
	'\0'
};

static uint8_t mem[4 * 1024 * 1024];

int main()
{
	Arena arena = arena_make(mem, sizeof(mem));
	
	PQ_Compiler c = {};

	{
		pq_init_compiler(&c, &arena, s(source), compiler_error_fn);
	
		pq_declare_foreign_proc(&c, s("print"), 1);
		pq_declare_foreign_proc(&c, s("sin"), 1);
		pq_declare_foreign_proc(&c, s("test"), 2);
	}

	PQ_CompiledBlob b = pq_compile(&c);

	PQ_VM vm = {};
	pq_init_vm(&vm, &arena, &b, vm_error_fn);
	
	dump_procedures(&c);
	dump_global_count(&c);
	dump_all_instructions(&c, &vm);

	println("Compiled size: %d bytes", b.size);

	pq_bind_foreign_proc(&vm, s("print"), print_proc);
	pq_bind_foreign_proc(&vm, s("sin"), sin_proc);
	pq_bind_foreign_proc(&vm, s("test"), test_proc);

	bool r = true;

	while (r)
	{
		//println("\n===============================");

		//dump_instruction(&c, &vm);

		r = pq_execute(&vm); 
		
		//dump_state(&vm);
		//dump_stack(&vm);
	}
}

#include <pq/compiler.c>

#include <pq/vm.c>