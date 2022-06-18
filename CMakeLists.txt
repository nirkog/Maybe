cmake_minimum_required(VERSION 3.20)

project(maybe)

add_library(maybe_lib STATIC
	src/test.c
)

target_include_directories(maybe_lib PUBLIC
	include/
)

add_subdirectory(sandbox)
