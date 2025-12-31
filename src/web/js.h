#pragma once

#include <stdint.h>

namespace js 
{

using Ref = __externref_t;
using RefHandle = uint32_t;

}

static js::Ref ref_table[0];

namespace js
{

static inline RefHandle push_ref(Ref r)
{
	return __builtin_wasm_table_grow(ref_table, r, 1);
}

static inline Ref load_ref(RefHandle h)
{
	return __builtin_wasm_table_get(ref_table, h);
}

static inline void store_ref(RefHandle h, Ref r)
{
	__builtin_wasm_table_set(ref_table, h, r);
}

extern "C"
{

//
// types
//

Ref get_window();

Ref get_document_body();

Ref null();

Ref obj();

Ref set_value(Ref, const char*, Ref);

float set_number(Ref, const char*, float);

void set_str(Ref, const char*, const char*);

Ref get_value(Ref, const char*);

float get_number(Ref, const char*);

void get_str(Ref, const char*, char*);

void to_lower(char*, const char*); 

//
// dom
//

Ref add_event_listener(Ref, const char*, void (*fn)(Ref));

Ref remove_event_listener(Ref, const char*, void (*fn)(Ref));

Ref get_element_by_id(const char*);

Ref create_element(const char*);

Ref create_element_ns(const char*, const char*);

Ref append_child(Ref, Ref);

void request_animation_frame(void (*fn)(double));

void console_log_obj(Ref);

void console_log(const char*);

void console_error(const char*);

int32_t set_timeout(void (*fn)(), float t);

void clear_timeout(int32_t);

void prevent_default(Ref);

void set_pointer_capture(Ref, int32_t);

void release_pointer_capture(Ref, int32_t);

void remove(Ref);

void set_attribute(Ref, const char*, const char*);

Ref get_computed_style(Ref);

//
// canvas 
//

Ref get_context(Ref, const char*, Ref);

void fill_rect(Ref, float, float, float, float);

void fill_text(Ref, const char*, float, float);

void clear_rect(Ref, float, float, float, float);

};

}

//
// libc
//

extern "C"
{

double atof(const char*);

}