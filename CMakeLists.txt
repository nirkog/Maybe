cmake_minimum_required(VERSION 3.20)

project(maybe)

# Include external scripts
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Windows build option
option(WINDOWS_BUILD "Compile for Windows" OFF)

# find_package(Vulkan)

include(FetchContent)
include(glfw)
# include(FindVulkan)

if(WINDOWS_BUILD)
set(PLATFORM_SPECIFIC_FILES
	src/logger/platforms/console/windows_console_logger.c
)
else()
set(PLATFORM_SPECIFIC_FILES
	src/logger/platforms/console/unix_console_logger.c
)
endif()

add_library(maybe_lib STATIC
	src/main.c
	src/utils/utils.c
	src/utils/io.c
	src/logger/logger.c
	src/logger/platforms/file/file_logger.c
	src/time/time.c
	src/common/list/list.c
	src/common/map/map.c
	src/common/vector/vector.c
	src/ecs/ecs.c
	src/ecs/archetype.c
	src/ecs/system.c
	src/graphics/graphics.c
	src/graphics/window.c
	src/graphics/vulkan/vulkan.c
	src/graphics/vulkan/extension_functions.c
	src/graphics/vulkan/swap_chain.c
	src/graphics/vulkan/queue.c
	src/graphics/vulkan/pipeline.c
	${PLATFORM_SPECIFIC_FILES}
)

target_include_directories(maybe_lib PUBLIC
	include/
	src/
)

target_include_directories(maybe_lib PRIVATE
	src/
)

target_compile_options(maybe_lib PRIVATE
	-g
)

# Vulkan
if(WINDOWS_BUILD)
	set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
	set(Vulkan_LIBRARIES 	"$ENV{VULKAN_SDK}/Lib")
	set(Vulkan_FOUND 		"True")
	target_include_directories(maybe_lib PUBLIC
		vendor/windows
	)
	target_link_directories(maybe_lib PUBLIC ${Vulkan_LIBRARIES})
	target_link_libraries(maybe_lib vulkan-1)
	add_definitions(-DWINDOWS_BUILD)
else()
	target_link_libraries(maybe_lib vulkan)
	target_compile_options(maybe_lib PRIVATE -Wall)
endif()

# GLFW
target_link_libraries(maybe_lib 
	glfw
)

# Build shaders
add_custom_command(
	TARGET maybe_lib PRE_BUILD
	COMMAND glslc ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.vert -o ${CMAKE_CURRENT_LIST_DIR}/shaders/vert.spv &&
			glslc ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.frag -o ${CMAKE_CURRENT_LIST_DIR}/shaders/frag.spv 
)

add_subdirectory(sandbox)

set(CMAKE_C_COMPILER /usr/bin/clang)
