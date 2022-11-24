#pragma once

#include <stdint.h>

#include "common/common.h"

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
