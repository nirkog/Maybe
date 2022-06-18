#include <stdio.h>
#include <stdint.h>

#include "common/error.h"

#include "file_logger.h"

maybe_error_t maybe_logger_platforms_file_init(
	maybe_logger_t* logger,
	void* params
) {
	return MAYBE_ERROR_SUCCESS;
}

maybe_error_t maybe_logger_platforms_file_write(
	maybe_logger_log_level_t log_level,
	uint8_t* data,
	uint32_t size,
	void* params
) {
	printf("%s", (const char*)data);	

	return MAYBE_ERROR_SUCCESS;
}
