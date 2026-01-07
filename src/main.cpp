#include <base/log.h>

#include <base/array.h>

#include <lang/tokenizer.h>

#include <lang/generator.h>

#include <lang/vm.h>

static Tokenizer tok;

static Generator gen;

static VM vm;

inline void print_tokens(Array <Token> tokens)
{
	println("\nTokens:");

	for (const Token& t : tokens)
	{
		char buf[128] = {};
		as_string(t, tok.source, buf, 128);
		
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

inline void print_instructions(Array <Instruction> instructions)
{
	println("\nInstructions:");

	uint16_t idx = 0;

	for (const Instruction& it : instructions)
	{
		if (it.type == INSTRUCTION_LOAD_IMMEDIATE)
		{
			char buf[128] = {};
			as_string(&gen.immediates[it.arg], buf, 128);

			println("  %-4d | %-20s %d (%s)", idx, to_string(it.type), it.arg, buf);
		}
		else if (it.type == INSTRUCTION_LOAD_LOCAL || it.type == INSTRUCTION_STORE_LOCAL)
		{
			println("  %-4d | %-20s %d (%s)", idx, to_string(it.type), it.arg, gen.variables[it.arg]);
		}
		else
		{
			println("  %-4d | %-20s %d", idx, to_string(it.type), it.arg);
		}

		idx++;
	}

	println("\nGenerated instructions' size: %llu bytes", instructions.count * sizeof(Instruction));
}

inline void dump_state() 
{
	println("  -----------");
	println("  Stack dump:");
	
	if (vm.stack.count > 0)
	{
		for (uint16_t i = 0; i < vm.stack.count; i++)
		{
			char buf[128] = {};
			as_string(&vm.stack[i], buf, 128);

			println("    [%d] %s", i, buf);
		}
	}
	else
	{
		println("    N/A");
	}

	println("");
}

inline void execute_program()
{
	println("\nExecution:");

	while (true)
	{
		const Instruction i = vm.instructions[vm.ic]; 
	
		println("  -> %-4d | %-20s %d", vm.ic, to_string(i.type), i.arg);
		
		Trap r = execute(&vm);

		dump_state();

		if (r == TRAP_HALT_EXECUTION)
		{
			println("\nVM: Execution halted.");

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

			errorln("VM: %s.", strings[r]);

			break;
		}
	}
}

static uint8_t arena_memory[128 * 1024];
static Arena arena;

static constexpr char source[] = 
R"(
define hello(x) {
	return x
}

var a = 10
var b = hello(a + 2)

a = b

)";

extern "C" void mainCRTStartup()
{
	arena = make_arena(arena_memory, sizeof(arena_memory));

	init(&tok, source);

	Array <Token> tokens = make_array<Token>(&arena, 1024);
	tokenize(&tok, &tokens);
	print_tokens(tokens);

	init(&gen, &arena, source, tokens);

	Array <Instruction> instructions = make_array<Instruction>(&arena, 1024);
	emit_program(&gen, &instructions);
	print_instructions(instructions);

	init(&vm, &arena, instructions, gen.immediates);
	execute_program();

	exit(0);
}

#include <lang/tokenizer.cpp>

#include <lang/generator.cpp>

#include <lang/vm.cpp>