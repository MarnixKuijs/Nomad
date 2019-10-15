cmake_minimum_required(VERSION 3.8.0)
project(Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(External/glm)

add_subdirectory(External/glfw)
set_target_properties(glfw
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/glfw/"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/glfw/"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/glfw/"
)

add_subdirectory(Nomad)
set_target_properties(Nomad
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Nomad/"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Nomad/"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Nomad/"
)

target_compile_options(Nomad PRIVATE /W4 -WX)

add_executable(COF main.cpp)

set_target_properties(COF
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/COF/"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/COF/"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/COF/"
)

target_include_directories(COF
    PUBLIC ./Nomad/Include/
	PUBLIC ./External/glfw/include/
	PUBLIC $ENV{VULKAN_SDK}/Include/
)

target_link_directories(COF
	PUBLIC $ENV{VULKAN_SDK}/Lib/
	PUBLIC ${CMAKE_BINARY_DIR}/bin/Nomad/
	PUBLIC ${CMAKE_BINARY_DIR}/bin/GLFW/
)

target_link_libraries(COF
	Nomad
	vulkan-1
	glfw3
)

target_compile_definitions(COF
	PRIVATE NOMINMAX
)
if(WIN32)
	target_compile_definitions(COF
		PRIVATE VK_USE_PLATFORM_WIN32_KHR
	)
elseif(UNIX AND NOT APPLE)
	target_compile_definitions(COF
		PRIVATE VK_USE_PLATFORM_XLIB_KHR
	)
endif()