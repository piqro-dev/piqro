#pragma once

#include <base/common.h>

struct Arena
{
	uint8_t* buffer;
	uint8_t* offset;

	size_t capacity;
};

inline Arena make_arena(uint8_t* buffer, size_t capacity) 
{
	Arena arena = {};

	arena.buffer = buffer;
	arena.offset = arena.buffer;
	arena.capacity = capacity;

	return arena;
}

template <typename T>
inline T* push(Arena* arena, size_t count = 1)
{
	ASSERT(arena->offset < arena->buffer + arena->capacity && "Out of memory");

	arena->offset = (uint8_t*)__builtin_align_up(arena->offset + sizeof(T) * count, alignof(T));
	return (T*)arena->offset;
}

template <typename T, typename ... Args>
inline T* emplace(Arena* arena, const Args& ... args)
{
	T* ptr = push<T>(arena);
	*ptr = T{ args... };
	return ptr;
}

inline void reset(Arena* arena)
{
	arena->offset = arena->buffer;
}