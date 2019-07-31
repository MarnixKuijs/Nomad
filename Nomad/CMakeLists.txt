cmake_minimum_required(VERSION 3.10.0)
project(Nomad VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_FILES
    ./Source/main.cpp
    ./Source/GPU/GPUContext.cpp
)

link_directories(
	$ENV{VULKAN_SDK}/Lib/
)

add_executable(Nomad ${SRC_FILES})

target_include_directories(Nomad
    PUBLIC ./Include/
	PUBLIC $ENV{VULKAN_SDK}/Include/
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



