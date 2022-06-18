#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/error.h"
#include "logger.h"

/*
 *  @brief Format a string that is being logged
 * */
bool format_string(
	uint8_t* format,
	uint32_t size,
	uint8_t** formatted_string,
	uint32_t* formatted_string_size,
	...
);

