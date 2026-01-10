#include <base/log.h>

#include <base/array.h>

#include <lang/compiler.h>

#include <lang/vm.h>

static constexpr char source[] = 
R"(
define fact(n)
{
	if n <= 1
	{
		return 1
	}

	return n * fact(n - 1)
}

var c = -1.
//var b = -fact(5)

)";

inline void print_tokens(const Tokenizer* tok, Array <Token> tokens)
{
	println("\nTokens:");

	for (const Token& t : tokens)
	{
		Identifier buf = {};
		as_string(t, tok->source, buf, 128);
		
		if (t.type >= TOKEN_STRING && t.type <= TOKEN_IDENTIFIER)
		{
			println("  %-15s %s", to_string(t.type), buf);
		}
		else
		{
			println("  %-15s", to_string(t.type));
		}
	}
}

inline void print_instructions(const Generator* gen, Array <Instruction> instructions)
{
	println("\nInstructions:");

	uint16_t idx = 0;

	for (const Instruction& it : instructions)
	{
		if (needs_argument(it.type))
		{
			if (it.type == INSTRUCTION_LOAD_IMMEDIATE)
			{
				char buf[128] = {};
				as_string(gen->immediates[it.arg], buf, 128);
	
				println("  %-4d | %-20s %d (%s)", idx, to_string(it.type), it.arg, buf);
			}
			else
			{
				println("  %-4d | %-20s %d", idx, to_string(it.type), it.arg);
			}
		}
		else
		{
			println("  %-4d | %-20s", idx, to_string(it.type));
		}

		idx++;
	}
}

inline void dump_state(const VM* vm) 
{
	println("  -----------");
	println("  Stack dump:");
	
	if (vm->stack.count > 0)
	{
		for (uint16_t i = 0; i < vm->stack.count; i++)
		{
			char buf[128] = {};
			as_string(vm->stack[i], buf, 128);

			println("    [%d] %s", i, buf);
		}
	}
	else
	{
		println("    N/A");
	}

	println("");
}

inline void execute_program(VM* vm)
{
	println("\nExecution:");

	while (true)
	{	
		Instruction it = vm->instructions[vm->ip];

		println("  -> %-4d | %-20s %d", vm->ip, to_string(it.type), it.arg);
		
		Trap r = execute(vm);

		dump_state(vm);

		if (r == TRAP_HALT_EXECUTION)
		{
			println("\nExecution halted.");

			break;
		}

		if (r != TRAP_SUCCESS)
		{
			constexpr const char* strings[] =
			{
				[TRAP_STACK_OVERFLOW]      = "Stack overflow",
				[TRAP_STACK_UNDERFLOW]     = "Stack underflow",
				[TRAP_OUT_OF_BOUNDS]       = "Out of bounds",
				[TRAP_ILLEGAL_INSTRUCTION] = "Illegal instruction"
			};

			errorln("Runtime Error: %s.", strings[r]);

			break;
		}
	}
}

static uint8_t arena_memory[2 * 1024 * 1024] = {};

void entry()
{
	Arena arena = make_arena(arena_memory, sizeof(arena_memory));
	
	Compiler com = {};
	init(&com, &arena, source);

	print_tokens(&com.tok, com.tokens);
	print_instructions(&com.gen, com.instructions);

	Array <uint8_t> blob = make_array<uint8_t>(&arena, MAX_BLOB_SIZE);
	compile_program(&com, &blob);

	VM vm = {};
	init(&vm, &arena, blob.elements, blob.count);

	execute_program(&vm);
}

#include <lang/tokenizer.cpp>

#include <lang/generator.cpp>

#include <lang/compiler.cpp>

#include <lang/vm.cpp>