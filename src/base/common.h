#pragma once

#include <stdint.h>

//
// common includes
//

#include <base/log.h>

//
// common types
//

using uvec2 = uint32_t __attribute__((ext_vector_type(2)));
using ivec2 = uint32_t __attribute__((ext_vector_type(2)));

using vec2 = float __attribute__((ext_vector_type(2)));
using vec4 = float __attribute__((ext_vector_type(4)));

//
// common macros
//

#define ASSERT(expr) do { if (!(expr)) { errorln("Assertion failed: %s, in file: %s, line: %d", #expr, __FILE__, __LINE__); __builtin_unreachable(); } } while (0) 

#define DO_ONCE for (static bool once = false; !once; once = true)

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))