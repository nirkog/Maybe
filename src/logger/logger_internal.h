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

/*
 * @brief Get all references to arguments in a format string
 * */
bool retrieve_argument_references(
	uint8_t* format,
	uint32_t size,
	argument_refernce_t* references,
	uint8_t* argument_types,
	uint8_t* argument_count,
	uint32_t* new_size
);

void replace_argument_references_with_values(
	uint8_t* format,
	uint32_t size,
	argument_refernce_t* references,
	uint8_t* formatted_string,
	uint32_t* int_argument_values,
	uint64_t* long_argument_values,
	double* float_argument_values
);
