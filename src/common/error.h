#pragma once

typedef enum {
	MAYBE_ERROR_UNINITIALIZED,
	MAYBE_ERROR_SUCCESS,

	MAYBE_ERROR_LOGGER_FORMAT_FAILED,
	MAYBE_ERROR_LOGGER_CONSOLE_ERROR,
	MAYBE_ERROR_LOGGER_FILE_ERROR,
	MAYBE_ERROR_LOGGER_FILE_INVALID_FILE_NAME,
	MAYBE_ERROR_LOGGER_FILE_WRITE_FAILED
} maybe_error_t;
