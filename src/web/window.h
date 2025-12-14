#pragma once

#include <base/common.h>

#include <web/js.h>

namespace window
{

static void (*on_resize)(js::Ref e);

static void (*on_mouse_down)(js::Ref e);

static void (*on_mouse_up)(js::Ref e);

static void (*on_mouse_move)(js::Ref e);

static void (*on_key_down)(js::Ref e);

static void (*on_key_up)(js::Ref e);

static vec2 inner_size;

static inline void init()
{
	inner_size = { js::get_number(js::get_window(), "innerWidth"), js::get_number(js::get_window(), "innerHeight") };

	js::add_event_listener(js::get_window(), "resize", [](js::Ref e)
	{
		inner_size = { js::get_number(js::get_window(), "innerWidth"), js::get_number(js::get_window(), "innerHeight") };

		if (on_resize)
		{
			on_resize(e);
		}
	});

	js::add_event_listener(js::get_window(), "mousedown", [](js::Ref e)
	{
		if (on_mouse_down)
		{
			on_mouse_down(e);
		}
	});

	js::add_event_listener(js::get_window(), "mouseup", [](js::Ref e)
	{
		if (on_mouse_up)
		{
			on_mouse_up(e);
		}
	});

	js::add_event_listener(js::get_window(), "mousemove", [](js::Ref e)
	{
		if (on_mouse_move)
		{
			on_mouse_move(e);
		}
	});

	js::add_event_listener(js::get_window(), "mousemove", [](js::Ref e)
	{
		if (on_mouse_move)
		{
			on_mouse_move(e);
		}
	});

	js::add_event_listener(js::get_window(), "keydown", [](js::Ref e)
	{
		if (on_key_down)
		{
			on_key_down(e);
		}
	});

	js::add_event_listener(js::get_window(), "keyup", [](js::Ref e)
	{
		if (on_key_up)
		{
			on_key_up(e);
		}
	});
}

}