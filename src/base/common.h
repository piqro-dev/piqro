#pragma once

// 
// system libraries
//

#include <stdint.h>

//
// system includes
//

#if !defined __wasm__
	#include <stdlib.h>
	#include <stdio.h>
#endif

//
// common includes
//

#include <base/log.h>

//
// common macros
//

#define ASSERT(expr) do { if (!(expr)) { errorln("Assertion failed: %s, in file %s, on line %d", #expr, __FILE__, __LINE__); __builtin_trap(); } } while (0) 

#define DO_ONCE for (static bool once = false; !once; once = true)

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

//
// entry point
//

void entry();

#if defined _WIN32
	extern "C" void mainCRTStartup()
#endif
{
	entry();

	exit(0);
}