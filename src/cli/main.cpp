#include <base/log.h>

#include <base/array.h>

#include <lang/state.h>

static constexpr char source[] = 
R"(
foreign define print(x)

var i = 0

repeat until i == 10 
{
	print(i)

	i += 1
}
)";

void print(VM* vm)
{
	char buf[256] = {};
	as_string(get_local(vm, 0), buf, 256);

	println("%s", buf);
}

static uint8_t arena_memory[2 * 1024 * 1024] = {};

int main(int argc, char** argv) 
{
	Arena arena = make_arena(arena_memory, sizeof(arena_memory));
	
	State state = {};
	init(&state, &arena, source);

	bind_procedure(&state, "print", print);

	evaluate(&state);
}

#include <lang/tokenizer.cpp>

#include <lang/generator.cpp>

#include <lang/compiler.cpp>

#include <lang/vm.cpp>

#include <lang/state.cpp>