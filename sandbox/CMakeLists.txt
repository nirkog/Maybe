cmake_minimum_required(VERSION 3.20)

project(maybe_sandbox)

add_executable(maybe_sandbox
	src/main.c
)

target_link_libraries(maybe_sandbox PRIVATE
	maybe_lib
)

target_compile_options(maybe_sandbox PRIVATE
	-Wall
	-g
)

set(CMAKE_C_COMPILER /usr/bin/clang)
