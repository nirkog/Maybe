cmake_minimum_required(VERSION 3.20)

project(maybe)

add_library(maybe_lib STATIC
	src/main.c
	src/logger/logger.c
	src/logger/platforms/console/console_logger.c
	src/logger/platforms/file/file_logger.c
)

target_include_directories(maybe_lib PUBLIC
	include/
)

target_include_directories(maybe_lib PRIVATE
	src/
)

target_compile_options(maybe_lib PRIVATE
	-Wall
)

add_subdirectory(sandbox)

set(CMAKE_C_COMPILER /usr/bin/clang)
