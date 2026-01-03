#include <base/log.h>

#include <base/array.h>

#include <lang/tokenizer.h>

#include <lang/generator.h>

static constexpr char src[] = R"(
define b() {
	return 10
}

var a = b()
)";

static Tokenizer tok;

static Generator gen;

static inline void print_tokens(Array <Token> tokens)
{
	println("\nToken:");

	for (const Token& t : tokens)
	{
		char buf[128] = {};
		as_string(t, src, buf, 128);
		
		if (t.type >= TOKEN_STRING && t.type <= TOKEN_IDENTIFIER)
		{
			println("\t%-15s %s", to_string(t.type), buf);
		}
		else
		{
			println("\t%-15s", to_string(t.type));
		}
	}
}

static inline void print_instructions(Array <Instruction> instructions)
{
	println("\nInstructions:");

	uint16_t idx = 0;

	for (const Instruction& it : instructions)
	{
		if (it.type == INSTRUCTION_LOAD_IMMEDIATE)
		{
			char buf[128] = {};
			as_string(&gen.immediates[it.param], buf, 128);

			println("\t[%d] %-20s %d (%s)", idx, to_string(it.type), it.param, buf);
		}
		else if (it.type == INSTRUCTION_LOAD_LOCAL || it.type == INSTRUCTION_STORE_LOCAL)
		{
			println("\t[%d] %-20s %d (%s)", idx, to_string(it.type), it.param, gen.variables[it.param]);
		}
		else
		{
			println("\t[%d] %-20s %d", idx, to_string(it.type), it.param);
		}

		idx++;
	}
}

static uint8_t arena_memory[128 * 1024];
static Arena arena;

extern "C" void mainCRTStartup()
{
	arena = make_arena(arena_memory, sizeof(arena_memory));

	init(&tok, src);

	Array <Token> tokens = make_array<Token>(&arena, 1024);
	tokenize(&tok, &tokens);
	print_tokens(tokens);

	init(&gen, &arena, src, tokens);

	Array <Instruction> instructions = make_array<Instruction>(&arena, 1024);
	emit_program(&gen, &instructions);
	print_instructions(instructions);

	exit(0);
}

#include <lang/tokenizer.cpp>

#include <lang/generator.cpp>

#include <lang/vm.cpp>