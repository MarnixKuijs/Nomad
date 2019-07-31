#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <vulkan/vulkan.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef VK_USE_PLATFORM_WIN32_KHR 

#define COF_OS_SURFACE_EXTENIONS_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XLIB_KHR

#define COF_OS_SURFACE_EXTENIONS_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME

#endif