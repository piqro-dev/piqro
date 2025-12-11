#include <core/core.h>

#include <web/js.h>

#include <web/canvas.h>

#include <web/window.h>

#include <block/vm.h>

#include <block/generator.h>

static VM vm;

static Generator gen;

static struct
{
	bool left;
	bool right;
	bool middle;

	vec2 client;
	vec2 movement;
} mouse;

static inline bool point_vs_rect(vec2 p, vec4 r)
{
	return p.x > r.x && p.y > r.y && p.x < r.x + r.z && p.y < r.y + r.w;
}

struct UIBlock
{
public:
	inline bool is_being_touched()
	{
		vec4 r;
		r.xy = position;
		r.zw = 50.0f;

		return point_vs_rect(mouse.client, r);
	}

	inline void update()
	{
		if (mouse.left)
		{
			if (is_being_touched() && !current)
			{
				current = this;
			}
		
			if (current == this)
			{
				position += mouse.movement;
			}
		}
	}

	inline void draw()
	{
		canvas::fill_rect(vec4{ position.x, position.y, 50.0f, 50.0f });
	}

public:
	static UIBlock* current;

public:
	vec2 position;
	Block* ptr;
};

static Array <UIBlock, MAX_BLOCKS> blocks;

static inline void update(double t)
{
	if (mouse.right)
	{
		blocks.push({ {}, Block::make(Opcode::ADD) });
	}

	for (uint16_t i = 0; i < blocks.count(); i++)
	{
		UIBlock& b = blocks[i];
		b.update();
	}

	if (!mouse.left)
	{
		UIBlock::current = nullptr;
	}
}

static inline void draw(double t)
{
	canvas::fill_style("darkblue");

	const vec4 s = { 0.0f, 0.0f, window::inner_size().x, window::inner_size().y };

	canvas::clear_rect(s);
	canvas::fill_rect(s);

	canvas::fill_style("rgb(%d, 255, 255)", 50 + blocks.count() * 25);
	canvas::font("%.fpx Arial", 16.0f + __builtin_fabsf(__builtin_sinf(t * 0.001f)) * 50.0f);

	canvas::fill_text(vec2{ 64.0f, 64.0f }, "fuck");

	for (uint16_t i = 0; i < blocks.count(); i++)
	{
		UIBlock& b = blocks[i];
		b.draw();
	}
};

static inline void loop(double t)
{
	update(t);

	draw(t);

	mouse.movement = 0.0f;

	js::request_animation_frame(loop);
}

int main() 
{
	window::init();

	canvas::init();

	canvas::set_size(window::inner_size());

	// register event handlers

	const auto mouse_handler = [](js::Ref e)
	{
		js::prevent_default(e);

		const uint8_t buttons = static_cast<uint8_t>(js::get_number(e, "buttons"));

		mouse.left = buttons & (1 << 0);
		mouse.right = buttons & (1 << 1);
		mouse.middle = buttons & (1 << 2);
	};

	window::on_mouse_down = mouse_handler;

	window::on_mouse_up = mouse_handler;

	window::on_mouse_move = [](js::Ref e)
	{
		mouse.movement = { js::get_number(e, "movementX"), js::get_number(e, "movementY") };
		mouse.client   = { js::get_number(e, "x"),js::get_number(e, "y") };
	};

	window::on_resize = [](js::Ref e)
	{
		canvas::set_size(window::inner_size());
	};

	js::request_animation_frame(loop);
}