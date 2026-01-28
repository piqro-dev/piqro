#include <runtime/state.h>

static RT_State* state;

#define DEFINE_RT_PROCEDURES \
	PROC(print, 1, \
	{ \
		Scratch scratch = scratch_make(vm->arena); \
		\
		printf("%.*s\n", s_fmt(pq_value_as_string(scratch.arena, pq_vm_get_local(vm, 0)))); \
		\
		scratch_release(scratch); \
		\
		pq_vm_return(vm); \
	}) \
	\
	PROC(clear, 0, \
	{ \
		rt_canvas_clear(&state->canvas); \
		\
		pq_vm_return(vm); \
	}) \
	PROC(present, 0, \
	{ \
		rt_canvas_present(&state->canvas); \
		\
		pq_vm_return(vm); \
	}) \
	PROC(line, 4, \
	{ \
		int16_t x0 = pq_value_as_number(pq_vm_get_local(vm, 0)); \
		int16_t y0 = pq_value_as_number(pq_vm_get_local(vm, 1)); \
		int16_t x1 = pq_value_as_number(pq_vm_get_local(vm, 2)); \
		int16_t y1 = pq_value_as_number(pq_vm_get_local(vm, 3)); \
		\
		rt_canvas_line(&state->canvas, x0, y0, x1, y1); \
		\
		pq_vm_return(vm); \
	}) \
	PROC(rect, 4, \
	{ \
		int16_t x = pq_value_as_number(pq_vm_get_local(vm, 0)); \
		int16_t y = pq_value_as_number(pq_vm_get_local(vm, 1)); \
		int16_t w = pq_value_as_number(pq_vm_get_local(vm, 2)); \
		int16_t h = pq_value_as_number(pq_vm_get_local(vm, 3)); \
		\
		rt_canvas_rect(&state->canvas, x, y, w, h); \
		\
		pq_vm_return(vm); \
	}) \
	PROC(put, 2, \
	{ \
		int16_t x = pq_value_as_number(pq_vm_get_local(vm, 0)); \
		int16_t y = pq_value_as_number(pq_vm_get_local(vm, 1)); \
		\
		rt_canvas_put(&state->canvas, x, y); \
		\
		pq_vm_return(vm); \
	}) \
	\
	PROC(abs, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_fabsf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(floor, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_floorf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(ceil, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_ceilf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(sqrt, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_sqrtf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(sin, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_sinf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(cos, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_cosf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(tan, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_tanf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(asin, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_asinf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(acos, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_acosf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(atan, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_atanf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(pow, 2, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_powf(pq_value_as_number(pq_vm_get_local(vm, 0)), pq_value_as_number(pq_vm_get_local(vm, 1))))); \
	}) \
	PROC(log, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_logf(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(log10, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(__builtin_log10f(pq_value_as_number(pq_vm_get_local(vm, 0))))); \
	}) \
	PROC(rad, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(pq_value_as_number(pq_vm_get_local(vm, 0)) * 0.0174533f)); \
	}) \
	PROC(deg, 1, \
	{ \
		pq_vm_return_value(vm, pq_value_number(pq_value_as_number(pq_vm_get_local(vm, 0)) * 57.2958f)); \
	})

#define PROC(name, arg_count, ...) \
	static void rt_proc_##name(PQ_VM* vm) \
	{	\
		__VA_ARGS__ \
	}

DEFINE_RT_PROCEDURES

//
// interface
//

void rt_state_init(Arena* arena, RT_State* s)
{
	s->arena = arena;

	s->canvas = rt_canvas_make(s->arena, RT_CANVAS_WIDTH, RT_CANVAS_HEIGHT);

	state = s;
}

#undef PROC
#define PROC(name, arg_count, ...) \
	pq_compiler_declare_foreign_proc((c), s(#name), (arg_count));

void rt_declare_procedures(PQ_Compiler* c)
{
	ASSERT(state);

	DEFINE_RT_PROCEDURES
}

#undef PROC
#define PROC(name, arg_count, ...) \
	pq_vm_bind_foreign_proc((vm), s(#name), rt_proc_##name);

void rt_bind_procedures(PQ_VM* vm)
{
	ASSERT(state);

	DEFINE_RT_PROCEDURES
}