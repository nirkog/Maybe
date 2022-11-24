#pragma once

#include <stdint.h>

#include "common/common.h"

/*
 * @brief Read a file and store its content in a newly allocated buffer
 *
 * @param path Path to the file
 * @param buffer A pointer to the new result buffer
 * @param size File content's size
 *
 * @note If the function succeeded, the buffer should be freed after use
 * */
maybe_error_t maybe_utils_io_read_file(
	const char* path,
	uint8_t** buffer,
	uint32_t* size
);
