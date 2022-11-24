#pragma once

#include <stdlib.h>

#include "error.h"
#include "logger/logger.h"

#define IS_FAILURE(result) (MAYBE_ERROR_SUCCESS != result)

#define DEBUG /* @TODO DO this well */

#ifdef DEBUG
#define LOG(msg, ...) maybe_logger_write(&g_maybe_logger, MAYBE_LOGGER_LOG_LEVEL_DEBUG, NULL, msg __VA_OPT__(,) __VA_ARGS__);
#endif

#define MALLOC_T(type, count) ((type*)(malloc(sizeof(type) * count)))

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#define CLEANUP_ON_FAILURE(call) result = (call); \
								 if (MAYBE_ERROR_SUCCESS != result) { \
									 goto l_cleanup; \
								 } \

#define GLFW_INCLUDE_VULKAN

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4101)
#pragma warning( pop )
#endif
