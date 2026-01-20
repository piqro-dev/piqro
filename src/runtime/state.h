#pragma once

#include <base/common.h>
#include <base/arena.h>

#include <runtime/config.h>

typedef struct RT_State RT_State;
struct RT_State
{
	Arena* arena;
	RT_Canvas canvas;
};

void rt_state_init(Arena* arena, RT_State* s);

void rt_declare_procedures(PQ_Compiler* c);

void rt_bind_procedures(PQ_VM* vm);
