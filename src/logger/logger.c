#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

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

	/* Format the message string */
	va_start(args, format);
	format_result = format_string(format, strlen((const char*)format), &formatted_message, &formatted_message_size, args);
	if (!format_result) {
		result = MAYBE_ERROR_LOGGER_FORMAT_FAILED;
		goto l_cleanup;
	}
	va_end(args);

	/* Output the message */
	result = logger_platforms[logger->platform].write(logger, formatted_message, formatted_message_size, params);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

bool format_string(
	uint8_t* format,
	uint32_t size,
	uint8_t** formatted_string,
	uint32_t* formatted_string_size,
	...
) {
	bool result = false;
	va_list args;
	uint32_t i;

	va_start(args, formatted_string_size);

	for (i = 0; i < size; i++) {

	}

	va_end(args);

	/* Copy the formatted string */
	*formatted_string_size = size;
	*formatted_string = (uint8_t*)malloc(size);
	memcpy((void*)*formatted_string, (void*)format, size);

	result = true;
l_cleanup:
	return result;
}
