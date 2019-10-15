cmake_minimum_required(VERSION 3.10.0)
project(Nomad VERSION 1.0.0)

set(SRC_FILES
    ./Source/GPU/GPUContext.cpp
	./Source/Graphics/Swapchain.cpp
	./Source/Graphics/Shader.cpp
)

add_library(Nomad ${SRC_FILES})

target_include_directories(Nomad
    PUBLIC ./Include/
	PUBLIC ../External/glfw/include/
	PUBLIC $ENV{VULKAN_SDK}/Include/
)

target_link_directories(Nomad
	PUBLIC $ENV{VULKAN_SDK}/Lib/

)

target_link_libraries(Nomad 
	vulkan-1
)
target_compile_definitions(Nomad
	PRIVATE NOMINMAX
)
if(WIN32)
	target_compile_definitions(Nomad
		PRIVATE VK_USE_PLATFORM_WIN32_KHR
	)
elseif(UNIX AND NOT APPLE)
	target_compile_definitions(Nomad
		PRIVATE VK_USE_PLATFORM_XLIB_KHR
	)
endif()



