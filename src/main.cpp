
#include "picovga.h"

// EGA (512x400) frame buffer for display
ALIGNED u8 Framebuffer[512*400];

int main()
{
	// Initialize video mode
	Video(DEV_VGA, RES_EGA, FORM_8BIT, Framebuffer);

	// Draw text
	DrawText(&Canvas, "Ciganyok", 0, 0,
		COL_WHITE, FontBoldB8x16, 16, 4, 4);

	DrawText(&Canvas, "nem emberek!", 0, 48,
		COL_WHITE, FontBoldB8x16, 16, 4, 4);
}
