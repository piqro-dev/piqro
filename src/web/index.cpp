#include <base/common.h>

#include <base/unicode.h>

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

static struct 
{
	bool control;
	bool shift;
	bool alt;
	bool escape;
	bool enter;
	bool backspace;

	bool keys[4096];
	bool repeated_keys[4096];

	inline bool down(char32_t k)
	{
		return repeated_keys[k];
	}

	inline bool up(char32_t k)
	{
		return !repeated_keys[k];
	}

	inline bool pressed(char32_t k)
	{
		return keys[k];
	}

	inline bool released(char32_t k)
	{
		return !keys[k];
	}
} keyboard;

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
	if (keyboard.pressed(L'w'))
	{
		static vec2 p = 0.0f;

		blocks.push({ { p.x, p.y }, Block::make(Opcode::ADD) });

		p.x += 75.0f;

		if (p.x > window::inner_size.x) 
		{
			p.x = 0;
			p.y += 75.0f;
		}
	}

	for (UIBlock& b : blocks)
	{
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

	const vec4 s = { 0.0f, 0.0f, window::inner_size.x, window::inner_size.y };

	canvas::clear_rect(s);
	canvas::fill_rect(s);

	canvas::fill_style("rgb(%d, 255, 255)", 50 + (blocks.count() * 10) % MAX_BLOCKS);
	canvas::font("%.fpx Arial", 16.0f + __builtin_fabsf(__builtin_sinf(t * 0.001f)) * 50.0f);

	//canvas::fill_text(vec2{ 64.0f, 64.0f }, "fuck");

	for (UIBlock& b : blocks)
	{
		b.draw();
	}
};

static inline void loop(double t)
{
	update(t);

	draw(t);

	mouse.movement = 0.0f;	

	// @hack: reset keys to false to make pressed() work, will fuck up
	// released() though. FIXME
	__builtin_memset(keyboard.keys, 0, sizeof(keyboard.keys));
	__builtin_memset(keyboard.repeated_keys, 0, sizeof(keyboard.repeated_keys));

	js::request_animation_frame(loop);
}

int main() 
{
	window::init();

	canvas::init();

	canvas::set_size(window::inner_size);

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
		mouse.client   = { js::get_number(e, "x"),         js::get_number(e, "y") };
	};

	window::on_resize = [](js::Ref e)
	{
		canvas::set_size(window::inner_size);
	};

	window::on_key_down = [](js::Ref e)
	{
		char key[32] = {};

		js::get_str(e, "key", key);
		js::to_lower(key, key);

		if (__builtin_strcmp(key, "control") == 0)
		{
			keyboard.control = true;
		}
		if (__builtin_strcmp(key, "shift") == 0)
		{
			keyboard.shift = true;
		}
		if (__builtin_strcmp(key, "alt") == 0)
		{
			keyboard.alt = true;
		}
		if (__builtin_strcmp(key, "escape") == 0)
		{
			keyboard.escape = true;
		}
		if (__builtin_strcmp(key, "enter") == 0)
		{
			keyboard.enter = true;
		}
		if (__builtin_strcmp(key, "backspace") == 0)
		{
			keyboard.backspace = true;
		}
		else
		{
			char32_t k;

			if (utf_8_to_32(key, &k))
			{
				if (static_cast<bool>(js::get_number(e, "repeat")))
				{
					keyboard.repeated_keys[k] = true;
					keyboard.keys[k] = false;
				}
				else
				{
					keyboard.keys[k] = true;
				}
			}
		}
		js::console_log_obj(e);
	};

	window::on_key_up = [](js::Ref e)
	{
		char key[32] = {};

		js::get_str(e, "key", key);
		js::to_lower(key, key);

		if (__builtin_strcmp(key, "control") == 0)
		{
			keyboard.control = false;
		}
		if (__builtin_strcmp(key, "shift") == 0)
		{
			keyboard.shift = false;
		}
		if (__builtin_strcmp(key, "alt") == 0)
		{
			keyboard.alt = false;
		}
		if (__builtin_strcmp(key, "escape") == 0)
		{
			keyboard.escape = false;
		}
		if (__builtin_strcmp(key, "enter") == 0)
		{
			keyboard.enter = false;
		}
		if (__builtin_strcmp(key, "backspace") == 0)
		{
			keyboard.backspace = false;
		}
		else
		{
			char32_t k;

			if (utf_8_to_32(key, &k))
			{
				keyboard.repeated_keys[k] = false;
				keyboard.keys[k] = false;
			}
		}
	};

	js::request_animation_frame(loop);
}