#include <lang/compiler.h>

template <typename T>
inline void write(Array <uint8_t>* blob, T v)
{
	__builtin_memcpy(blob->elements + blob->count, &v, sizeof(T));
	blob->count += sizeof(T);
}

inline void write_magic(Compiler* com)
{
	write<uint32_t>(com->blob, __builtin_bswap32('PIQR'));
}

inline void write_immediates(Compiler* com)
{
	write<uint8_t>(com->blob, com->gen.immediates.count);

	for (const Value& v : com->gen.immediates)
	{
		write<ValueType>(com->blob, v.type);

		switch (v.type)
		{
			case VALUE_UNDEFINED: break;

			case VALUE_NUMBER:  write<float>(com->blob, v.number); break;
			case VALUE_BOOLEAN: write<bool>(com->blob, v.boolean); break;

			case VALUE_STRING:
			{
				const char* s = v.string;

				while (s++)
				{
					write<char>(com->blob, *s);
				}

				write<char>(com->blob, '\0');
			} break;
		}
	}
}

inline void write_procedures(Compiler* com)
{
	write<uint8_t>(com->blob, com->gen.procedures.count);

	for (const Procedure& p : com->gen.procedures)
	{
		write<bool>(com->blob, p.foreign);
		write<uint8_t>(com->blob, p.local_count);
		write<uint8_t>(com->blob, p.arg_count);
		write<uint16_t>(com->blob, p.scope.first_inst);
	}
}

inline void write_instructions(Compiler* com)
{
	write<uint16_t>(com->blob, com->instructions.count);

	for (const Instruction& i : com->instructions)
	{
		write<uint8_t>(com->blob, i.type);

		if (needs_argument(i.type))
		{
			write<uint16_t>(com->blob, i.arg);
		}
	}
}

void init(Compiler* com, Arena* arena, const char* source)
{
	com->source = source;

	// tokenize the source code
	init(&com->tok, com->source);

	com->tokens = make_array<Token>(arena, MAX_TOKENS);
	tokenize(&com->tok, &com->tokens);

	// generate the instructions, procedures, immediates, etc.
	init(&com->gen, arena, com->source, com->tokens);

	com->instructions = make_array<Instruction>(arena, MAX_INSTRUCTIONS);
	emit_program(&com->gen, &com->instructions);
}

void compile_program(Compiler* com, Array <uint8_t>* blob)
{
	com->blob = blob;

	write_magic(com);

	write_immediates(com);
	write_procedures(com);

	write_instructions(com);
}