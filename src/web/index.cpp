#include <core/core.h>

#include <web/js.h>

#include <block/vm.h>

static VM vm;

int main() 
{
	
	/*
	Value immediates[] =
	{
		Value(10.0f),
		Value(1.0f),
	};
	Instruction instructions[] = 
	{
		Instruction(Instruction::LOAD_IMMEDIATE, 1),
		Instruction(Instruction::LOAD_IMMEDIATE, 0),
		Instruction(Instruction::LOAD_IMMEDIATE, 1),
		Instruction(Instruction::CALL, 5),
		Instruction(Instruction::ADD),
		Instruction(Instruction::JUMP, 10),
		Instruction(Instruction::LOAD_IMMEDIATE, 0),
		Instruction(Instruction::LOAD_LOCAL, 0),
		Instruction(Instruction::MUL),
		Instruction(Instruction::RET),
		Instruction(Instruction::NOOP),
	};*/

	Value immediates[] =
	{
		Value(6.0f),
		Value(1.0f),
		Value(2.0f),
	};

	Instruction instructions[] = 
	{
		Instruction(Instruction::JUMP, 5),

		Instruction(Instruction::LOAD_NULL),
		Instruction(Instruction::CALL, 1),
		Instruction(Instruction::LOAD_NULL),
		Instruction(Instruction::RET),
		
		Instruction(Instruction::LOAD_NULL),
		Instruction(Instruction::CALL, 1),

		Instruction(Instruction::NOOP),
	};
/*
	Instruction instructions[] = 
	{
		// jump to entry point
		Instruction(Instruction::JUMP, 23),

		// function fib(n)
		// if (n == 0 || n == 1)
		Instruction(Instruction::LOAD_LOCAL, 0), // n
		Instruction(Instruction::LOAD_NULL), // 0
		Instruction(Instruction::EQUALS),

		Instruction(Instruction::LOAD_LOCAL, 0), // n
		Instruction(Instruction::LOAD_IMMEDIATE, 1), // 1
		Instruction(Instruction::EQUALS),

		Instruction(Instruction::OR),

		Instruction(Instruction::JUMP_COND, 11),

		// return n;
		Instruction(Instruction::LOAD_LOCAL, 0),
		Instruction(Instruction::RET),

		// fib(n - 1)
		Instruction(Instruction::LOAD_LOCAL, 0), // n
		Instruction(Instruction::LOAD_IMMEDIATE, 1), // 1
		Instruction(Instruction::SUB),
		Instruction(Instruction::LOAD_IMMEDIATE, 1), // argc = 1
		Instruction(Instruction::CALL, 1), // call fib()

		// fib(n - 2)
		Instruction(Instruction::LOAD_LOCAL, 0), // n
		Instruction(Instruction::LOAD_IMMEDIATE, 2), // 2
		Instruction(Instruction::SUB),
		Instruction(Instruction::LOAD_IMMEDIATE, 1), // argc = 1
		Instruction(Instruction::CALL, 1), // call fib()

		// add the two function calls together
		Instruction(Instruction::ADD),
		Instruction(Instruction::RET),

		// fib(6)
		Instruction(Instruction::LOAD_IMMEDIATE, 0), // n
		Instruction(Instruction::LOAD_IMMEDIATE, 1), // argc = 1
		Instruction(Instruction::CALL, 1), // call fib()

		Instruction(Instruction::NOOP)
	};
*/
	vm.init(instructions, COUNT_OF(instructions), immediates, COUNT_OF(immediates));
	
	VM::Trap r;

	while (!vm.is_done())
	{
		const Instruction& i = vm.current_instruction();

		println("[%d] %s: %d", vm.ic(), i.as_string(), i.param);
		println("Call frame depth: %zu", vm.call_frames().count());

		r = vm.execute();
	
		if (r != VM::SUCCESS)
		{
			constexpr const char* fmt[] =
			{
				[VM::SUCCESS] = "SUCCESS",
				[VM::STACK_OVERFLOW] = "STACK_OVERFLOW",
				[VM::STACK_UNDERFLOW] = "STACK_UNDERFLOW",
				[VM::OUT_OF_BOUNDS] = "OUT_OF_BOUNDS",
				[VM::ILLEGAL_INSTRUCTION] = "ILLEGAL_INSTRUCTION",
			};
	
			println("VM Execution has failed: %s", fmt[r]);

			break;
		}

		vm.dump_stack();
	}
}