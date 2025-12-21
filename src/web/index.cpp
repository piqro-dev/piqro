#include <base/log.h>

#include <web/js.h>

int main() 
{
	js::Ref button = js::get_element_by_id("compile-button");

	js::add_event_listener(button, "click", [](js::Ref)
	{
		println("yo");
	});
}