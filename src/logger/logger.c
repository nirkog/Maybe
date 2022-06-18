#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "logger.h"
#include "logger_internal.h"

#include "common/common.h"
#include "platforms/console/console_logger.h"
#include "platforms/file/file_logger.h"

/*
 * @brief The implementations of the current logging platforms
 * */
static maybe_logger_platform_t logger_platforms[] = {
	[MAYBE_LOGGER_PLATFORM_TYPE_FILE] = { maybe_logger_platforms_file_init, maybe_logger_platforms_file_write },
	[MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE] = { maybe_logger_platforms_console_init, maybe_logger_platforms_console_write },
};

maybe_error_t maybe_logger_init(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	maybe_logger_platform_type_t platform,
	void* params
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	/* Initialize logger state */
	logger->log_level = log_level;
	logger->platform = platform;
	logger->platform_state = NULL;

	/* Call the platform-specific initializer */
	result = logger_platforms[platform].init(logger, params);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_logger_write(
	maybe_logger_t* logger,
	void* params,
	const char* format,
	...
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	bool format_result = false;
	va_list args;
	uint8_t* formatted_message = NULL;
	uint32_t formatted_message_size = 0;

	va_start(args, format);

	/* Format the message string */
	format_result = format_string((uint8_t*)format, strlen((const char*)format), &formatted_message, &formatted_message_size, args);
	if (!format_result) {
		result = MAYBE_ERROR_LOGGER_FORMAT_FAILED;
		goto l_cleanup;
	}

	/* Output the message */
	result = logger_platforms[logger->platform].write(logger, formatted_message, formatted_message_size, params);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	va_end(args);

	return result;
}

bool retrieve_argument_references(
	uint8_t* format,
	uint32_t size,
	argument_refernce_t* references,
	uint8_t* argument_types,
	uint8_t* argument_count,
	uint32_t* new_size
) {
	bool result = false;
	uint8_t reference_count = 0;
	uint32_t i;

	/* @TODO Handle edge cases */
	for (i = 0; i < size; i++) {
		if (format[i] == '{') {
			if ((i == 0) || (format[i - 1] != '\\')) {
				if (format[i + 3] != '}') {
					goto l_cleanup;
				}

				references[reference_count].index = format[i + 1] - '0';
				references[reference_count].type = format[i + 2];
				references[reference_count].position = i;

				argument_types[references[reference_count].index] = references[reference_count].type;

				if ((references[reference_count].index + 1) > *argument_count) {
					*argument_count = references[reference_count].index + 1;
				}

				switch (references[reference_count].type) {
				case 'i':
					new_size += 5;
					break;
				case 'f':
					new_size += 15;
					break;
				case 'l':
					new_size += 20;
					break;
				default:
					goto l_cleanup;
				}

				reference_count++;
				if (reference_count >= MAX_FORMAT_ARGUMENT_REFERENCE_COUNT) {
					goto l_cleanup;
				}

			}
		}
	}

	result = true;
l_cleanup:
	return result;
}

void replace_argument_references_with_values(
	uint8_t* format,
	uint32_t size,
	argument_refernce_t* references,
	uint8_t* formatted_string,
	uint32_t* int_argument_values,
	uint64_t* long_argument_values,
	double* float_argument_values
) {
	uint32_t i;
	uint8_t current_reference = 0;

	for (i = 0; i < size; i++) {
		if (i == references[current_reference].position) {
			switch (references[current_reference].type) {
			case 'i':
				formatted_string += sprintf((char*)formatted_string, "%d", (int)int_argument_values[references[current_reference].index]);
				break;
			case 'f':
				formatted_string += sprintf((char*)formatted_string, "%f", (float)float_argument_values[references[current_reference].index]);
				break;
			case 'l':
				formatted_string += sprintf((char*)formatted_string, "%lld", (long long)long_argument_values[references[current_reference].index]);
				break;
			}
			
			i += 3;
			current_reference++;
		} else {
			*formatted_string = format[i];
			formatted_string++;
		}
	}
}

bool format_string(
	uint8_t* format,
	uint32_t size,
	uint8_t** formatted_string,
	uint32_t* formatted_string_size,
	va_list args
) {
	bool result = false;
	uint32_t i, new_size = size;
	uint8_t* _formatted_string = NULL;
	uint8_t argument_count = 0;
	uint32_t int_argument_values[MAX_FORMAT_ARGUMENT_COUNT] = { 0 };
	uint64_t long_argument_values[MAX_FORMAT_ARGUMENT_COUNT] = { 0 };
	double float_argument_values[MAX_FORMAT_ARGUMENT_COUNT] = { 0.0f };
	uint8_t types[MAX_FORMAT_ARGUMENT_COUNT] = { 0 };
	argument_refernce_t references[MAX_FORMAT_ARGUMENT_REFERENCE_COUNT] = { { 0 } };

	/* @TODO Optimize and tidy this */

	result = retrieve_argument_references(format, size, references, types, &argument_count, &new_size);
	if (!result) {
		goto l_cleanup;
	}

	/* Extract arguments */
	for (i = 0; i < argument_count; i++) {
		switch (types[i]) {
		case 'i':
			int_argument_values[i] = va_arg(args, uint32_t);
			break;
		case 'f':
			float_argument_values[i] = va_arg(args, double);
			break;
		case 'l':
			long_argument_values[i] = va_arg(args, uint64_t);
			break;
		default:
			int_argument_values[i] = va_arg(args, uint32_t);
			break;
		}
	}

	_formatted_string = (uint8_t*)malloc(new_size + 1);
	_formatted_string[new_size] = '\0';

	replace_argument_references_with_values(format, size, references, _formatted_string, int_argument_values, long_argument_values, float_argument_values);

	/* Transfer the result */
	*formatted_string_size = new_size;
	*formatted_string = _formatted_string;

	result = true;
l_cleanup:
	return result;
}
