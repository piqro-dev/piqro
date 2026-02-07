#include "pico/stdlib.h"
#include "ili9341.h"

int main()
{
	stdio_init_all();

	lcd_set_pins(16, 18, 17, 10, 11);
	lcd_init();
	lcd_set_rotation(1);
	
	lcd_fill_screen(ILI9341_BLACK);
	lcd_fill_rect(0, 0, 50, 50, ILI9341_WHITE);
	lcd_fill_rect(100, 100, 50, 50, ILI9341_RED);
	lcd_present();

	int x = 0;
	int y = 100;
	int dx = 2;
	int dy = 2;

	while (1)
	{
		lcd_fill_screen(ILI9341_BLACK);
		
		lcd_fill_rect(x, y, 30, 30, ILI9341_CYAN);
		
		x += dx;
		y += dy;
		
		if (x <= 0 || x >= 290)
		{
			dx = -dx;
		}
		if (y <= 0 || y >= 210)
		{
			dy = -dy;
		}
		
		lcd_present();
		
		sleep_ms(16);
	}

	return 0;
}