#include <base/log.h>

#include <lang/tokenizer.h>

#include <lang/generator.h>

#include <lang/vm.h>

static constexpr char src[] = 
R"(
repeat 50 {
	var a = 10
}
)";

int main() 
{
	Tokenizer tok(src);

	Array <Token, 1024> tokens;
	tok.tokenize(tokens);

	println("\nTokens:");

	for (const Token& t : tokens)
	{
		char buf[128] = {};
		t.value(src, buf, 128);
		
		if (t.type >= Token::STRING_LIT && t.type <= Token::IDENTIFIER)
		{
			println("\t%-20s %s", t.as_string(), buf);
		}
		else
		{
			println("\t%-20s", t.as_string());
		}
	}

	Generator gen(src, tokens.begin(), tokens.count());

	Array <Instruction, 2048> instructions;
	gen.emit_program(instructions);

	gen.dump_immediates();
	gen.dump_variables();

	println("\nProgram:");

	for (const Instruction& it : instructions)
	{
		println("\t%-20s %hu", it.as_string(), it.param);
	}
/*
	VM vm(instructions.begin(), instructions.count(), gen.immediates().begin(), gen.immediates().count());

	println("\nExecution:");

	do 
	{
		vm.dump_stack();

		const Instruction& inst = vm.current_instruction();

		println("\n-> %s, %hu\n", inst.as_string(), inst.param);
	} while (vm.execute() == VM::SUCCESS);*/

	return 0;
}

#if defined _WIN32
	extern "C" void mainCRTStartup()
	{
		ExitProcess(main());
	}
#endif

#include <lang/tokenizer.cpp>

#include <lang/generator.cpp>

#include <lang/vm.cpp>