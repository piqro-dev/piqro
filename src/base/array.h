#pragma once

#include <base/common.h>

#include <base/arena.h>

template <typename T>
struct Array
{
	T* elements = nullptr;

	size_t count = 0;
	size_t capacity = 0;

	inline T& operator[](size_t idx) { return elements[idx]; }
	inline const T& operator[](size_t idx) const { return elements[idx]; }
};

template <typename T>
static inline Array <T> make_array(Arena* arena, size_t capacity)
{
	Array <T> array = {};

	array.capacity = capacity;
	array.elements = push<T>(arena, array.capacity);

	return array;
}

template <typename T>
static inline T* push(Array <T>* array, const T& v)
{
	return &(array->elements[array->count++] = v);
}

template <typename T>
static inline T* push(Array <T>* array)
{
	return &array->elements[array->count++];
}

template <typename T, typename ... Args>
static inline T* emplace(Array <T>* array, const Args& ... args)
{
	return &(array->elements[array->count++] = T{ args... });
}

template <typename T>
static inline T* pop(Array <T>* array)
{
	return &array->elements[--array->count];
}

// Both begin() and end() need references to the type...

template <typename T>
static inline T* begin(Array <T>& array)
{
	return array.elements;	
}

template <typename T>
static inline T* end(Array <T>& array)
{
	return array.elements + array.count;
}	

template <typename T>
static inline void trim_end(Array <T>* array, size_t from)
{
	array->count = from;
}

template <typename T>
static inline T* remove(Array <T>* array, size_t at)
{
	return &array->elements[at] = *pop(array);
}