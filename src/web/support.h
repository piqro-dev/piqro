#pragma once

#include <base/common.h>

//
// helpers
//

static __externref_t ref_table[0];

static inline uint32_t push_ref(__externref_t r)
{
	return __builtin_wasm_table_grow(ref_table, r, 1);
}

static inline __externref_t load_ref(uint32_t h)
{
	return __builtin_wasm_table_get(ref_table, h);
}

static inline void store_ref(uint32_t h, __externref_t r)
{
	__builtin_wasm_table_set(ref_table, h, r);
}

//
// libc functions implemented in javascript
//

extern double atof(const char*);

extern float fmodf(float, float);

//
// javascript imports
//

extern void js_console_log(const char*);

extern void js_console_error(const char*);

extern void js_alert(const char*);

extern __externref_t js_get_element_by_id(const char*);

extern void js_set_string(__externref_t, const char*, const char*);

extern void js_add_event_listener(__externref_t, const char*, void (*)(__externref_t e));

extern int64_t js_get_int(__externref_t, const char*);

extern void js_get_string(__externref_t, const char*, char*);

extern __externref_t js_get(__externref_t, const char*);

extern __externref_t js_get_context(__externref_t, const char*);

extern void js_clear_rect(__externref_t, uint32_t, uint32_t, uint32_t, uint32_t);

extern void js_fill_rect(__externref_t, uint32_t, uint32_t, uint32_t, uint32_t);