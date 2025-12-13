#pragma once

#include <base/common.h>

#include <ui/window.h>

#include <ui/os.h>

#include <ui/gfx.h>

extern "C" void mainCRTStartup()
{
	window::init("piqro");

	gfx::init();

	window::on_resize = []
	{
		gfx::resize_viewport();
	};

	window::on_paint = []
	{
		gfx::begin();
		gfx::clear(0.3f, 0.2f, 0.3f, 1.0f);
		gfx::present();
	};

	while (window::open)
	{
		window::poll_events();
	}

	os::exit(0);
}