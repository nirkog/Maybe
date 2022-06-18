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
 * @param data The data to output
 * @param size The size of the data in bytes
 * @param params Parameters for file output
 * */
maybe_error_t maybe_logger_platforms_file_write(
	maybe_logger_t* logger,
	uint8_t* data,
	uint32_t size,
	void* params
);
