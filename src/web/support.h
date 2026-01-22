#pragma once

#include <base/common.h>

//
// libc functions implemented in javascript
//

extern double atof(const char*);

extern void puts(const char*);

//
// javascript imports
//

extern void js_alert(const char*);

extern void js_post_message(__externref_t);

extern void js_console_log(const char*);
extern void js_console_error(const char*);

extern __externref_t js_string(const char*);
extern __externref_t js_obj();

extern void js_set_int(__externref_t, const char*, int32_t);
extern void js_set_string(__externref_t, const char*, const char*);

extern int32_t js_get_int(__externref_t, const char*);
extern void js_get_string(__externref_t, const char*, char*);
extern __externref_t js_get(__externref_t, const char*);

//
// helpers
//

static __externref_t __ref_table[0];

static inline uint32_t web_push_ref(__externref_t r)
{
	return __builtin_wasm_table_grow(__ref_table, r, 1);
}

static inline __externref_t web_load_ref(uint32_t h)
{
	return __builtin_wasm_table_get(__ref_table, h);
}

static inline void web_store_ref(uint32_t h, __externref_t r)
{
	__builtin_wasm_table_set(__ref_table, h, r);
}

__attribute__((format(printf, 1, 2)))
static inline int printf(const char* fmt, ...)
{
	char out[4096];

	va_list args;

	va_start(args, fmt);
	int s = vsnprintf(out, sizeof(out), fmt, args);
	va_end(args);

	puts(out);

	return s;
}