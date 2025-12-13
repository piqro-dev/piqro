#include <ui/window.h>

#include <ui/os.h>

namespace window
{

static inline LRESULT window_proc(HWND hwnd, UINT u_msg, WPARAM wparam, LPARAM lparam)
{
	switch (u_msg) 
	{
		case WM_DESTROY:
		{
			open = false;
		} break;

		case WM_CLOSE: 
		{
			DestroyWindow(hwnd);
		} break;
		
		case WM_ERASEBKGND: 
		{
			return 1;
		} break;
		
		case WM_MOUSEMOVE: 
		{
			mouse.client.x = GET_X_LPARAM(lparam);
			mouse.client.y = GET_Y_LPARAM(lparam);
		} break;

		case WM_ENTERSIZEMOVE:
		{
		} break;

		case WM_EXITSIZEMOVE:
		{
		} break;

		case WM_SIZE:
		{
			size = { LOWORD(lparam), HIWORD(lparam) };

			if (on_resize)
			{
				on_resize();
			}
		};

		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			BeginPaint(hwnd, &ps);
			
			if (on_paint)
			{
				on_paint();
			}

			EndPaint(hwnd, &ps);
		} break;
		
		default: 
		{
			return DefWindowProcW(hwnd, u_msg, wparam, lparam);
		} break;
	}

	return 0;
}

static WNDCLASSEXW wnd_class;

static inline void init(const char* title)
{
	wnd_class = 
	{
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = window_proc,
		.hInstance = GetModuleHandleW(nullptr),
		.lpszClassName = L"piqro",
		.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW),
	};

	if (!RegisterClassExW(&wnd_class))
	{
		os::dialog("Failed to register class.", "Fatal error", os::ICON_ERROR);
		os::exit(1);
	}

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

	wchar_t w_title[512];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, w_title, 512);

	handle = static_cast<void*>(CreateWindowW(wnd_class.lpszClassName, w_title, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE, CW_USEDEFAULT, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wnd_class.hInstance, nullptr));

	if (!handle)
	{
		os::dialog("Failed to create graphical window.", "Fatal error", os::ICON_ERROR);
		os::exit(1);
	}

	open = true;
}

static inline void poll_events()
{
	MSG msg;
	
	while (PeekMessageW(&msg, static_cast<HWND>(handle), 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

}