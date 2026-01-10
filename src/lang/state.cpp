#include <lang/state.h>

void init(State* state, Arena* arena, const char* source)
{
	state->source = source;

	init(&state->com, arena, source);

	state->blob = make_array<uint8_t>(arena, MAX_BLOB_SIZE);
	compile(&state->com, &state->blob);

	init(&state->vm, arena, state->blob.elements, state->blob.count);
}

void bind_procedure(State* state, const char* name, NativeProcedure proc)
{
	for (const Procedure& p : state->com.gen.procedures)
	{
		if (__builtin_strcmp(p.name, name) == 0)
		{
			bind_procedure(&state->vm, p.idx, proc);
		}
	}
}

void evaluate(State* state)
{
	while (true)
	{	
		Trap r = execute(&state->vm);

		if (r == TRAP_HALT_EXECUTION)
		{
			break;
		}

		if (r != TRAP_SUCCESS)
		{
			errorln("Runtime Error: %s.", to_string(r));
			break;
		}
	}
}