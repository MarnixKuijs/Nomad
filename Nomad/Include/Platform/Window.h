namespace cof
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
#include "Windows/Win32Window.h"
	using Window = cof::internal::Win32Window;
#elif VK_USE_PLATFORM_XLIB_KHR
#include "Linux/XLibWindow.h"
	using Window = cof::internal::XLibWindow;
#endif
}

