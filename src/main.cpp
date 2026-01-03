#include <base/log.h>

#include <lang/tokenizer.h>

#include <lang/generator.h>

#include <lang/vm.h>

static constexpr char src[] = 
R"(
define hello(x, y) {
	
}

hello(10, 10)

)";

int main() 
{
	Tokenizer tok;
	tok.init(src);

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

	Generator gen;
	gen.init(src, tokens.begin(), tokens.count());

	Array <Instruction, 2048> instructions;
	gen.emit_program(instructions);

	println("\nProgram:");

	for (size_t i = 0; i < instructions.count(); i++)
	{
		const Instruction& it = instructions[i];

		if (it.type == Instruction::LOAD_IMMEDIATE)
		{
			char buf[128] = {};
			gen.immediates()[it.param].as_string(buf, 128);

			println("[%zu]\t%-20s %hu (%s)", i, it.as_string(), it.param, buf);
		}
		else if (it.type == Instruction::LOAD_LOCAL || it.type == Instruction::STORE_LOCAL)
		{
			println("[%zu]\t%-20s %hu (%s)", i, it.as_string(), it.param, gen.variables()[it.param]);
		}
		else
		{
			println("[%zu]\t%-20s %hu", i, it.as_string(), it.param);
		}
	}

	/*VM vm(instructions.begin(), instructions.count(), gen.immediates().begin(), gen.immediates().count());

	println("\nExecution:");

	do 
	{
	} while (vm.execute() == VM::SUCCESS);*/

	return 0;
}

#if defined _WIN32
	extern "C" void mainCRTStartup()
	{
		exit(main());
	}
#endif

#include <lang/tokenizer.cpp>

#include <lang/generator.cpp>