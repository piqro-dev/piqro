#include <ui/os.h>

namespace os
{

static inline void exit(int32_t exit_code)
{
	ExitProcess(exit_code);
}

static inline void dialog(const char* message, const char* caption, uint32_t flags)
{
	uint32_t mb_flags = 0;

	if (flags & DialogFlags::ICON_ERROR)
		mb_flags |= MB_ICONERROR;

	if (flags & DialogFlags::ICON_INFO)
		mb_flags |= MB_ICONINFORMATION;

	wchar_t w_message[1024];
	MultiByteToWideChar(CP_UTF8, 0, message, -1, w_message, 1024);

	wchar_t w_caption[256];
	MultiByteToWideChar(CP_UTF8, 0, caption, -1, w_caption, 256);

	MessageBoxW(nullptr, w_message, w_caption, mb_flags);
}

}