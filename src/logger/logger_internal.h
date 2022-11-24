#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/error.h"
#include "logger.h"

#define MAX_FORMAT_ARGUMENT_REFERENCE_COUNT (30)
#define MAX_FORMAT_ARGUMENT_COUNT (10)

#define TIME_PROMPT_SIZE (13)
#define TIME_PROMPT_FORMAT ("[%02d:%02d:%04d] ")

#define UNSIGNED_INT_FORMAT_SPECIFIER ('u')
#define SIGNED_INT_FORMAT_SPECIFIER ('i')
#define DOUBLE_FORMAT_SPECIFIER ('d')
#define UNSIGNED_LONG_FORMAT_SPECIFIER ('l')
#define SIGNED_LONG_FORMAT_SPECIFIER ('q')
#define STRING_FORMAT_SPECIFIER ('s')
#define HEX_FORMAT_SPECIFIER ('x')
#define LONG_HEX_FORMAT_SPECIFIER ('X')

typedef struct {
	uint32_t position;
	uint8_t index;
	uint8_t type;
} argument_refernce_t;

typedef union {
	uint32_t _uint32_t;
	double _double;
	uint64_t _uint64_t;
	char* _char_pointer;
} argument_value_t;

/*
 * @brief Format a string that is being logged
 * */
bool format_string(
	maybe_logger_log_level_t log_level,
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
	uint8_t* reference_count,
	uint8_t* argument_types,
	uint8_t* argument_count
);

/*
 * @brief Calculate the formatted size of a format string and its arguments
 * */
uint32_t calculate_formatted_size(
	uint32_t format_size,
	argument_refernce_t* references,
	uint32_t reference_count,
	argument_value_t* arguments
);

/*
 * @brief Replace all occurrences of argument references with argument values
 * */
uint32_t replace_argument_references_with_values(
	uint8_t* format,
	uint32_t size,
	argument_refernce_t* references,
	uint8_t reference_count,
	uint8_t* formatted_string,
	argument_value_t* argument_values
);

extern maybe_error_t maybe_logger_platforms_console_init(
	maybe_logger_t* logger,
	void* params
);

extern maybe_error_t maybe_logger_platforms_console_write(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	uint8_t* data,
	uint32_t size,
	void* params
);

extern void maybe_logger_platforms_console_free(
	maybe_logger_t* logger
);
