#pragma once

#include <base/common.h>

typedef struct
{
	uint8_t* buffer;
	size_t offset;
	size_t capacity;
} Arena;

static inline Arena arena_make(uint8_t* buffer, size_t capacity)
{
	Arena arena = {};

	arena.buffer = buffer;
	arena.offset = 0;
	arena.capacity = capacity;

	return arena;
}

static inline void* _arena_push(Arena* arena, size_t size, size_t align)
{
	size_t offset = __builtin_align_up(arena->offset, align);
	
	if (offset + size > arena->capacity)
	{
		return nullptr;
	}

	arena->offset = offset + size;

	uint8_t* ptr = arena->buffer + offset;
	__builtin_memset(ptr, 0, size);

	return ptr;
}

static inline void arena_reset(Arena* arena)
{
	arena->offset = 0;
}

#define arena_push(arena, T) ((T*)_arena_push((arena), sizeof(T), alignof(T)))

#define arena_push_array(arena, T, N) ((T*)_arena_push((arena), sizeof(T) * (N), alignof(T)))

typedef struct
{
	Arena* arena;
	size_t offset;
} Scratch;

static inline Scratch scratch_make(Arena* arena)
{
	Scratch scratch = {};

	scratch.arena = arena;
	scratch.offset = scratch.arena->offset;

	return scratch;
}

static inline void scratch_release(Scratch scratch)
{
	scratch.arena->offset = scratch.offset;
}