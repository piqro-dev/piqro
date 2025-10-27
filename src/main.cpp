#include "picovga.h"

static uint8_t screen_buffer[512 * 400];

void set_pixel(uint16_t x, uint16_t y, uint8_t c)
{
	screen_buffer[x + y * 512] = c;
}

int main()
{
	Video(DEV_VGA, RES_EGA, FORM_8BIT, screen_buffer);

	float v = 0.0f;

	while (true)
	{
		v += 0.01f;

		WaitVSync();

		__builtin_memset(screen_buffer, 255, sizeof(screen_buffer));
		
		for (size_t i = 0; i < 512; i++)
		{
			set_pixel(i, 200 + (__builtin_sinf(v + i * 0.1f) * 10.0f), 67);
		}


	}
}
