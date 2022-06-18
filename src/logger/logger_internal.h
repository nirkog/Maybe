#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/error.h"
#include "logger.h"

#define MAX_FORMAT_ARGUMENT_REFERENCE_COUNT (30)
#define MAX_FORMAT_ARGUMENT_COUNT (10)

typedef struct {
	uint32_t position;
	uint8_t index;
	uint8_t type;
} argument_refernce_t;

/*
 * @brief Format a string that is being logged
 * */
bool format_string(
	uint8_t* format,
	uint32_t size,
	uint8_t** formatted_string,
	uint32_t* formatted_string_size,
	va_list args
);

