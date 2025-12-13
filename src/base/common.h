#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//
// system includes
//

#if defined _WIN32
	#include <windows.h>
	#include <windowsx.h>

	#include <d3d11.h>
	#include <dxgi.h>
#endif

//
// system libraries
//

#if defined _WIN32
	#pragma comment(lib, "vcruntime")
	#pragma comment(lib, "msvcrt")
	#pragma comment(lib, "ucrt")
	#pragma comment(lib, "kernel32")
	#pragma comment(lib, "user32")

	#pragma comment(lib, "d3d11")
	#pragma comment(lib, "dxgi")
	#pragma comment(lib, "dxguid")
#endif

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