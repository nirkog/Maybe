cmake_minimum_required(VERSION 3.20)

project(maybe)

# Include external scripts
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)
include(glfw)

add_library(maybe_lib STATIC
	src/main.c
	src/logger/logger.c
	src/logger/platforms/console/console_logger.c
	src/logger/platforms/file/file_logger.c
	src/time/time.c
	src/common/list/list.c
	src/common/map/map.c
	src/common/vector/vector.c
	src/ecs/ecs.c
	src/ecs/archetype.c
	src/ecs/system.c
)

target_include_directories(maybe_lib PUBLIC
	include/
	src/
)

target_include_directories(maybe_lib PRIVATE
	src/
)

target_compile_options(maybe_lib PRIVATE
	-Wall
	-g
)

# GLFW
target_link_libraries(maybe_lib glfw)

add_subdirectory(sandbox)

option(WINDOWS_BUILD "Compile for Windows" OFF)
if(WINDOWS_BUILD)
	target_include_directories(maybe_lib PRIVATE
		vendor/windows
	)
endif(WINDOWS_BUILD)

set(CMAKE_C_COMPILER /usr/bin/clang)
