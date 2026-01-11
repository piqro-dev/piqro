#pragma once

#include <base/common.h>

#include <base/arena.h>

#define Array(T) \
	struct \
	{ \
		T* elements; \
		\
		size_t count; \
		size_t capacity; \
	}

#define array_make(arena, T, N) \
	({ \
		Array (T) array = {}; \
		\
		array.capacity = (N); \
		array.count = 0; \
		array.elements = arena_push_array((arena), T, array.capacity); \
		\
		array; \
	})

#define array_push(array, v) \
	({ \
		ASSERT((array)->count < (array)->capacity); \
		&(array)->elements[(array)->count++] = (v); \
	})

#define array_emplace(array, ...) \
	({ \
		ASSERT((array)->count < (array)->capacity); \
		&((array)->elements[(array)->count++] = (typeof((array)->elements[0])){ __VA_ARGS__ }); \
	})

#define array_pop(array) \
	({ \
		ASSERT((array)->count > 0); \
		&(array)->elements[--(array)->count]; \
	})

#define array_peek(array) \
	({ \
		&(array)->elements[(array)->count - 1]; \
	})

#define array_trim_end(array, from) \
	({ \
		(array)->count = from; \
	})