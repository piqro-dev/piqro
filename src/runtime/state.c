#include <runtime/state.h>

static RT_State* g_state;

#define DEFINE_RT_PROCEDURES \
	PROC(print, 1, \
	{ \
		Scratch scratch = scratch_make(vm->arena); \
		printf("%.*s\n", s_fmt(pq_value_as_string(scratch.arena, pq_vm_get_local(vm, 0)))); \
		scratch_release(scratch); \
		pq_vm_return(vm); \
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

	s->canvas = canvas_make(s->arena, RT_CANVAS_WIDTH, RT_CANVAS_HEIGHT);

	g_state = s;
}

#undef PROC
#define PROC(name, arg_count, ...) \
	pq_compiler_declare_foreign_proc((c), s(#name), (arg_count));

void rt_declare_procedures(PQ_Compiler* c)
{
	ASSERT(g_state);

	DEFINE_RT_PROCEDURES
}

#undef PROC
#define PROC(name, arg_count, ...) \
	pq_vm_bind_foreign_proc((vm), s(#name), rt_proc_##name);

void rt_bind_procedures(PQ_VM* vm)
{
	ASSERT(g_state);

	DEFINE_RT_PROCEDURES
}