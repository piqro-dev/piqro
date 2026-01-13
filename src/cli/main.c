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

static constexpr const char source[] = 
{
	#embed "test_bed.pq" 
	,'\0'
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
	}

	PQ_CompiledBlob b = pq_compile(&c);

	PQ_VM vm = {};
	pq_init_vm(&vm, &arena, &b, vm_error_fn);
	
	//dump_all_instructions(&c, &vm);

	pq_bind_foreign_proc(&vm, s("print"), print_proc);
	pq_bind_foreign_proc(&vm, s("sin"), sin_proc);

	bool r = true;

	while (r)
	{
		//dump_instruction(&c, &vm);

		r = pq_execute(&vm); 

		//dump_stack(&vm);
	}
}

#include <pq/compiler.c>

#include <pq/vm.c>