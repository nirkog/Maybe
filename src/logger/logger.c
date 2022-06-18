#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

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
	uint8_t* format,
	...
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	bool format_result = false;
	va_list args;
	uint8_t* formatted_message = NULL;
	uint32_t formatted_message_size = 0;

	va_start(args, format);

	/* Format the message string */
	format_result = format_string(format, strlen((const char*)format), &formatted_message, &formatted_message_size, args);
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
	uint8_t reference_count = 0;
	uint8_t current_reference = 0;
	uint8_t* formatted_string_pointer = NULL;
	uint8_t argument_count = 0;
	uint32_t int_argument_values[MAX_FORMAT_ARGUMENT_COUNT] = { 0 };
	double float_argument_values[MAX_FORMAT_ARGUMENT_COUNT] = { 0.0f };
	uint8_t types[MAX_FORMAT_ARGUMENT_COUNT] = { 0 };
	argument_refernce_t references[MAX_FORMAT_ARGUMENT_REFERENCE_COUNT] = { { 0 } };

	/* @TODO Handle edge cases */
	/* @TODO Export this to a function */
	for (i = 0; i < size; i++) {
		if (format[i] == '{') {
			if ((i == 0) || (format[i - 1] != '\\')) {
				if (format[i + 3] != '}') {
					goto l_cleanup;
				}

				references[reference_count].index = format[i + 1] - '0';
				references[reference_count].type = format[i + 2];
				references[reference_count].position = i;

				types[references[reference_count].index] = references[reference_count].type;

				if ((references[reference_count].index + 1) > argument_count) {
					argument_count = references[reference_count].index + 1;
				}

				switch (references[reference_count].type) {
				case 'i':
					new_size += 5;
					break;
				case 'f':
					new_size += 10;
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

	/* Extract arguments */
	for (i = 0; i < argument_count; i++) {
		switch (types[i]) {
		case 'i':
			int_argument_values[i] = va_arg(args, uint32_t);
			break;
		case 'f':
			float_argument_values[i] = va_arg(args, double);
			break;
		default:
			int_argument_values[i] = va_arg(args, uint32_t);
			break;
		}
	}

	/* @TODO Transfer this to a function */
	_formatted_string = (uint8_t*)malloc(new_size + 1);
	_formatted_string[new_size] = '\0';
	formatted_string_pointer = _formatted_string;

	for (i = 0; i < size; i++) {
		if (i == references[current_reference].position) {
			switch (references[current_reference].type) {
			case 'i':
				formatted_string_pointer += sprintf(formatted_string_pointer, "%d", (int)int_argument_values[references[current_reference].index]);
				break;
			case 'f':
				formatted_string_pointer += sprintf(formatted_string_pointer, "%f", (float)float_argument_values[references[current_reference].index]);
				break;
			}
			
			i += 3;
			current_reference++;
		} else {
			*formatted_string_pointer = format[i];
			formatted_string_pointer++;
		}
	}

	/* Transfer the result */
	*formatted_string_size = new_size;
	*formatted_string = _formatted_string;

	result = true;
l_cleanup:
	return result;
}
