#pragma once

//
// system includes
//

#include <stdarg.h>
#include <stdint.h>

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

#define ASSERT(expr) do { if (!(expr)) { printf("Assertion failed: %s, in file %s, on line %d", #expr, __FILE__, __LINE__); __builtin_trap(); } } while (0) 

#define DO_ONCE for (static bool once = false; !once; once = true)

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

#define MIN(a, b) (__builtin_elementwise_min((a), (b)))

#define MAX(a, b) (__builtin_elementwise_max((a), (b)))

#define CLAMP(x, min, max) (MIN((typeof((x)))(max), MAX((x), (typeof((x)))(min))))