#pragma once

//
// system includes
//

#include <stdarg.h>

#if !defined __wasm__
	#include <stdlib.h>
	#include <stdio.h>
#endif

//
// third party includes
//

#if defined __wasm__
	#define STB_SPRINTF_IMPLEMENTATION
	#define STB_SPRINTF_DECORATE(name) name

	#include <third_party/stb_sprintf.h>
#endif

//
// macros
//

#define ASSERT(expr) do { if (!(expr)) { printf("Assertion failed: %s, in file %s, on line %d", #expr, __FILE__, __LINE__); __debugbreak(); } } while (0) 

#define DO_ONCE for (static bool once = false; !once; once = true)

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

#define MIN(a, b) (__builtin_elementwise_min((a), (b)))

#define MAX(a, b) (__builtin_elementwise_max((a), (b)))

//
// types
//

#if defined __wasm__
	typedef long int64_t;
	typedef unsigned long uint64_t;
	typedef unsigned long size_t;
#else
	typedef long long int64_t;
	typedef unsigned long long uint64_t;
	typedef unsigned long long size_t;
#endif

typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;