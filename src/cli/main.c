#include <base/log.h>

#include <pq/compiler.h>

#include <pq/vm.h>

void compiler_error_fn(uint16_t line, const char* fmt, ...)
{
	va_list args;
	
	char out[2048];
	
	va_start(args, fmt);

	int len = sprintf(out, "Compilation error: line %d: ", line);
	len += vsprintf(out + len, fmt, args);

	va_end(args);

	errorln("%s", out);

	exit(1);
}

void vm_error_fn(const char* fmt, ...)
{
	va_list args;
	
	char out[2048];
	
	va_start(args, fmt);

	int len = sprintf(out, "Runtime error: ");
	len += vsprintf(out + len, fmt, args);

	va_end(args);

	errorln("%s", out);

	exit(1);
}

void print_proc(PQ_VM* vm)
{
	String v = pq_value_as_string(vm->arena, pq_vm_get_local(vm, 0));

	println("%.*s", s_fmt(v));

	pq_vm_push(vm, pq_value_undefined());
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

static uint8_t mem[4 * 1024 * 1024];

int main()
{
	Arena arena = arena_make(mem, sizeof(mem));

	String source = s("f define print(x) var i = 0 repeat until i == 10 { print(i) i += 1 }");
	
	PQ_Compiler c = {};
	pq_compiler_init(&c, &arena, source, compiler_error_fn);

	PQ_CompiledBlob b = pq_compile(&c);

	PQ_VM vm = {};
	pq_vm_init(&vm, &arena, &b, vm_error_fn);
	
	for (uint16_t i = 0; i < vm.instruction_count; i++)
	{
		PQ_Instruction it = vm.instructions[i];

		if (pq_inst_needs_arg(it.type))
		{
			println("  %-4d | %-20s %d", i, pq_inst_to_c_str(it.type), it.arg);
		}
		else
		{
			println("  %-4d | %-20s", i, pq_inst_to_c_str(it.type));
		}
	}

	pq_vm_bind_procedure(&vm, 0, print_proc);

	println("");

	while (true)
	{
		PQ_Instruction it = vm.instructions[vm.ip];

		//if (pq_inst_needs_arg(it.type))
		//{
		//	printf("-> %-3d | %-20s %d", vm.ip, pq_inst_to_c_str(it.type), it.arg);
		//}
		//else
		//{
		//	printf("-> %-3d | %-20s", vm.ip, pq_inst_to_c_str(it.type));
		//}

		PQ_Trap r = pq_vm_execute(&vm); 

		//dump_stack(&vm);

		if (r == TRAP_HALT_EXECUTION)
		{
			break;
		}

		if (r != TRAP_SUCCESS)
		{
			vm_error_fn("%s", pq_trap_to_c_str(r));
			break;
		}
	}
}

#include <pq/compiler.c>

#include <pq/vm.c>