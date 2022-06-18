#pragma once

#include <stdint.h>

#include "logger/logger.h"

/*
 * @brief Initialize the console logger
 *
 * @param logger The logger
 * @param params Currently this logger need no parameters
 * */
maybe_error_t maybe_logger_platforms_file_init(
	maybe_logger_t* logger,
	void* params
);

/*
 * @brief Write data to the console
 *
 * @param logger The logger
 * @param log_level The message's log level
 * @param data The data to output
 * @param size The size of the data in bytes
 * @param params Parameters for file output
 * */
maybe_error_t maybe_logger_platforms_file_write(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	uint8_t* data,
	uint32_t size,
	void* params
);

/*
 * @brief Free a file logger's resources
 *
 * @param logger The logger
 * */
void maybe_logger_platforms_file_free(
	maybe_logger_t* logger
);
