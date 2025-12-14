#pragma once

#include <base/common.h>

#include <web/js.h>

namespace canvas
{

static js::RefHandle canvas_ref;
static js::RefHandle ctx_ref;

static inline void init()
{
	canvas_ref = js::push_ref(js::append_child(js::get_document_body(), js::create_element("canvas")));

	{
		js::Ref options = js::obj();
	
		js::set_number(options, "alpha", false);
		
		ctx_ref = js::push_ref(js::get_context(js::load_ref(canvas_ref), "2d", options));
	}
}

static inline void set_size(const vec2 s)
{
	js::set_number(js::load_ref(canvas_ref), "width", s.x);
	js::set_number(js::load_ref(canvas_ref), "height", s.y);
}

static inline void fill_style(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	js::set_str(js::load_ref(ctx_ref), "fillStyle", out);
}

static inline void font(const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	js::set_str(js::load_ref(ctx_ref), "font", out);
}

static inline void fill_text(vec2 p, const char* fmt, ...)
{
	__builtin_va_list args;

	char out[2048];

	__builtin_va_start(args, fmt);
	__builtin_vsprintf(out, fmt, args);
	__builtin_va_end(args);

	js::fill_text(js::load_ref(ctx_ref), out, p.x, p.y);
}

static inline void clear_rect(vec4 r)
{
	js::clear_rect(js::load_ref(ctx_ref), r.x, r.y, r.z, r.w);
}

static inline void fill_rect(vec4 r)
{
	js::fill_rect(js::load_ref(ctx_ref), r.x, r.y, r.z, r.w);
}

}