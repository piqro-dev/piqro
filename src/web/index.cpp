#include <core/core.h>

#include <web/js.h>

#include <block/vm.h>

#include <block/generator.h>

static VM vm;

static Generator gen;

static js::RefHandle canvas;
static js::RefHandle ctx;

static vec2 screen_size;

static vec2 mouse_position;
static vec2 mouse_delta;

static bool mouse_down;

static Array <Block*, MAX_BLOCKS> blocks;

static inline void update()
{
	for (uint16_t i = 0; i < blocks.count(); i++)
	{
		Block* b = blocks[i];

		if (mouse_down && mouse_position.x > b->position.x && mouse_position.y > b->position.y && mouse_position.x < b->position.x + 50.0f && mouse_position.y < b->position.y + 50.0f)
		{
			b->position += mouse_delta;
		}
	}
}

static inline void draw()
{
	js::clear_rect(js::load_ref(ctx), 0.0f, 0.0f, screen_size.x, screen_size.y);

	js::fill_text(js::load_ref(ctx), "fuck", 64.0f, 64.0f);

	for (uint16_t i = 0; i < blocks.count(); i++)
	{
		const Block* b = blocks[i];

		js::fill_rect(js::load_ref(ctx), b->position.x, b->position.y, 50.0f, 50.0f);
	}
}

static inline void loop(double t)
{
	update();
	draw();

	js::request_animation_frame(loop);
}

static inline void on_mouse_move(js::Ref e)
{
	int32_t timer = 0;

	mouse_delta = { js::get_number(e, "movementX"), js::get_number(e, "movementY") };
	mouse_position = { js::get_number(e, "clientX"), js::get_number(e, "clientY") };
	
	if (timer)
	{
		js::clear_timeout(timer);
	}
	
	timer = js::set_timeout([] 
	{
		js::remove_event_listener(js::window(), "mousemove", on_mouse_move);
		mouse_delta = 0.0f;
	}, 500.0f);

	js::add_event_listener(js::window(), "mousemove", on_mouse_move);
}

int main() 
{
	blocks.push(Block::make(Opcode::ADD));

	canvas = js::push_ref(js::append_child(js::document_body(), js::create_element("canvas")));

	ctx = js::push_ref(js::get_context(js::load_ref(canvas), "2d"));

	screen_size.x = js::get_number(js::window(), "innerWidth");
	screen_size.y = js::get_number(js::window(), "innerHeight");

	js::set_number(js::load_ref(canvas), "width", screen_size.x);
	js::set_number(js::load_ref(canvas), "height", screen_size.y);

	js::add_event_listener(js::window(), "resize", [](js::Ref e)
	{
		screen_size.x = js::get_number(js::window(), "innerWidth");
		screen_size.y = js::get_number(js::window(), "innerHeight");

		js::set_number(js::load_ref(canvas), "innerWidth", screen_size.x);
		js::set_number(js::load_ref(canvas), "innerHeight", screen_size.y);
	});

	js::add_event_listener(js::window(), "mousedown", [](js::Ref e)
	{
		mouse_down = true;	
	});

	js::add_event_listener(js::window(), "mouseup", [](js::Ref e)
	{
		mouse_down = false;	
	});

	js::add_event_listener(js::window(), "mousemove", on_mouse_move);

	js::request_animation_frame(loop);

	/*Array <Instruction, 128> insts;

	Block* var = Block::make(Opcode::SET_VAR);
		var->append_child(Block::make(Opcode::VALUE, "hello"));
		var->append_child(Block::make(Opcode::VALUE, 80.0f));

	gen.emit_set_variable(var, insts);

	Block* add = Block::make(Opcode::ADD);
		Block* get = Block::make(Opcode::GET_VAR);
		get->append_child(Block::make(Opcode::VALUE, "hello"));

		add->append_child(get);
		add->append_child(Block::make(Opcode::VALUE, 75.0f));

	gen.emit_binary_op(add, insts);

	vm.init(insts.data(), insts.count(), gen.immediates().data(), gen.immediates().count());

	VM::Trap r;

	while (!vm.is_done())
	{
		const Instruction& i = vm.current_instruction();

		println("[%d] %s: %d", vm.ic(), i.as_string(), i.param);
		println("Call frame depth: %zu", vm.call_frames().count());

		r = vm.execute();
	
		if (r != VM::SUCCESS)
		{
			constexpr const char* fmt[] =
			{
				[VM::SUCCESS] = "SUCCESS",
				[VM::STACK_OVERFLOW] = "STACK_OVERFLOW",
				[VM::STACK_UNDERFLOW] = "STACK_UNDERFLOW",
				[VM::OUT_OF_BOUNDS] = "OUT_OF_BOUNDS",
				[VM::ILLEGAL_INSTRUCTION] = "ILLEGAL_INSTRUCTION",
			};
	
			println("VM Execution has failed: %s", fmt[r]);

			break;
		}

		vm.dump_stack();
	}*/
}