#include <stdio.h>
#include <stdint.h>

#include "common/error.h"

#include "console_logger.h"

maybe_error_t maybe_logger_platforms_console_init(
	maybe_logger_t* logger,
	void* params
) {
	return MAYBE_ERROR_SUCCESS;
}

maybe_error_t maybe_logger_platforms_console_write(
	maybe_logger_t* logger,
	uint8_t* data,
	uint32_t size,
	void* params
) {
	printf("%s", (const char*)data);	

	return MAYBE_ERROR_SUCCESS;
}
