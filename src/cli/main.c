#include <pq/compiler.h>
#include <pq/vm.h>

void compiler_error_fn(uint16_t line, const char* what)
{
	printf("Compilation error: line %d: %s\n", line, what);
	exit(1);
}

void vm_error_fn(const char* what)
{
	printf("Runtime error: %s\n", what);
	exit(1);
}

void print_proc(PQ_VM* vm)
{
	Scratch scratch = scratch_make(vm->arena);

	String v = pq_value_as_string(scratch.arena, pq_vm_get_local(vm, 0));

	printf("%.*s\n", s_fmt(v));

	scratch_release(scratch);

	pq_vm_return(vm);
}

void sin_proc(PQ_VM* vm)
{
	float v = pq_value_as_number(pq_vm_get_local(vm, 0));

	pq_vm_return_value(vm, pq_value_number(__builtin_sinf(v)));
}

void test_proc(PQ_VM* vm)
{
	Scratch scratch = scratch_make(vm->arena);

	String name = pq_value_as_string(scratch.arena, pq_vm_get_local(vm, 1));

	if (pq_value_as_boolean(pq_vm_get_local(vm, 0)))
	{
		printf("TEST: [SUCCESS] %.*s\n", s_fmt(name));
	}
	else
	{
		printf("TEST: [FAILURE] %.*s\n", s_fmt(name));
	}

	scratch_release(scratch);

	pq_vm_return(vm);
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
			else if (it.type == INST_LOAD_GLOBAL)
			{
				printf("  %-4d | %-25s %d (%.*s)\n", i, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->globals[it.arg].name));
			}
			else if (it.type == INST_STORE_GLOBAL)
			{
				printf("  %-4d | %-25s %d (%.*s)\n", i, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->globals[it.arg].name));
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

void dump_instruction(PQ_Compiler* c, PQ_VM* vm)
{
	PQ_Instruction it = vm->instructions[vm->ip];

	Scratch scratch = scratch_make(c->arena);
	
	if (pq_inst_needs_arg(it.type))
	{
		if (it.type == INST_CALL)
		{
			printf("\n-> %d | %-25s %d (%.*s)\n", vm->ip, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->procedures[it.arg].name));
		}
		else if (it.type == INST_LOAD_IMMEDIATE)
		{
			printf("\n-> %d | %-25s %d (%.*s)\n", vm->ip, pq_inst_to_c_str(it.type), it.arg, s_fmt(pq_value_as_string(scratch.arena, c->immediates[it.arg])));
		}
		else if (it.type == INST_LOAD_GLOBAL)
		{
			printf("\n-> %d | %-25s %d (%.*s)\n", vm->ip, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->globals[it.arg].name));
		}
		else if (it.type == INST_STORE_GLOBAL)
		{
			printf("\n-> %d | %-25s %d (%.*s)\n", vm->ip, pq_inst_to_c_str(it.type), it.arg, s_fmt(c->globals[it.arg].name));
		}
		else
		{
			printf("\n-> %d | %-25s %d\n", vm->ip, pq_inst_to_c_str(it.type), it.arg);
		}
	}
	else
	{
		printf("\n-> %d | %-25s\n", vm->ip, pq_inst_to_c_str(it.type));
	}

	scratch_release(scratch);
}

void dump_global_count(PQ_Compiler* c)
{
	printf("global count: %d\n", c->global_count);
	printf("\n");
}

void dump_state(PQ_VM* vm)
{	
	printf("\nVM info:\n");
	printf("local count:      %d\n", vm->local_count);
	printf("stack size:       %d\n", vm->stack_size);
	printf("call frame count: %d\n", vm->call_frame_count);

	printf("\nVM memory consumption: %d bytes\n", vm->arena->offset);
}

static constexpr const char source[] = 
{
	#embed "test_bed.pq" 
	,
	'\0'
};

int main()
{
	// NOTE: this amount of memory is  sufficient to compile any program that's smaller than PQ_MAX_BLOB_SIZE.
	static uint8_t compiler_mem[4 * 1024 * 1024];
	
	Arena compiler_arena = arena_make(compiler_mem, sizeof(compiler_mem));
	
	PQ_Compiler c = {};

	{
		pq_compiler_init(&c, &compiler_arena, s(source), compiler_error_fn);
	
		pq_compiler_declare_foreign_proc(&c, s("print"), 1);
		pq_compiler_declare_foreign_proc(&c, s("sin"), 1);
		pq_compiler_declare_foreign_proc(&c, s("test"), 2);
	}

	PQ_CompiledBlob b = pq_compile(&c);

	static uint8_t vm_mem[128 * 1024];
	Arena vm_arena = arena_make(vm_mem, sizeof(vm_mem));

	PQ_VM vm = {};

	dump_procedures(&c);
	dump_instructions(&c);

	{
		pq_vm_init(&vm, &vm_arena, &b, vm_error_fn);
	
		pq_vm_bind_foreign_proc(&vm, s("print"), print_proc);
		pq_vm_bind_foreign_proc(&vm, s("sin"), sin_proc);
		pq_vm_bind_foreign_proc(&vm, s("test"), test_proc);
	
		while (pq_execute(&vm)) 
		{
			//dump_instruction(&c, &vm);
			//dump_state(&vm);
			//dump_stack(&vm);
		}
	}
}

#include <pq/compiler.c>
#include <pq/vm.c>