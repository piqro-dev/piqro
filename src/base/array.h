#pragma once

#include <base/common.h>

#include <base/arena.h>

template <typename T>
struct Array
{
	T* elements = nullptr;

	size_t count = 0;
	size_t capacity = 0;

	inline const T& operator[](size_t idx) const { return elements[idx]; }
	inline T& operator[](size_t idx)             { return elements[idx]; }
};

template <typename T>
inline Array <T> make_array(Arena* arena, size_t capacity)
{
	Array <T> array = {};

	array.capacity = capacity;
	array.elements = push<T>(arena, array.capacity);

	return array;
}

template <typename T>
inline T* push(Array <T>* array, T v)
{
	ASSERT(array->count < array->capacity);

	return &(array->elements[array->count++] = v);
}

template <typename T>
inline T* push(Array <T>* array)
{
	ASSERT(array->count < array->capacity);

	return &array->elements[array->count++];
}

template <typename T, typename ... Args>
inline T* emplace(Array <T>* array, Args ... args)
{
	ASSERT(array->count < array->capacity);

	return &(array->elements[array->count++] = T{ args... });
}

template <typename T>
inline T* pop(Array <T>* array)
{
	return &array->elements[--array->count];
}

template <typename T>
inline void trim_end(Array <T>* array, size_t from)
{
	array->count = from;
}

template <typename T>
inline T* remove(Array <T>* array, size_t at)
{
	return &array->elements[at] = *pop(array);
}

// Both begin() and end() need references to the type...

template <typename T>
inline T* begin(Array <T>& array)
{
	return array.elements;	
}

template <typename T>
inline T* end(Array <T>& array)
{
	return array.elements + array.count;
}	

template <typename T>
inline T* begin(const Array <T>& array)
{
	return array.elements;	
}

template <typename T>
inline T* end(const Array <T>& array)
{
	return array.elements + array.count;
}	