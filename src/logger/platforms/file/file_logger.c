#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/error.h"

#include "file_logger.h"
#include "file_logger_internal.h"

maybe_error_t maybe_logger_platforms_file_init(
	maybe_logger_t* logger,
	void* params
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint8_t* original_file_name = (uint8_t*)params;
	uint32_t original_file_name_size = 0;
	internal_state_t* state = NULL;

	/* Validate a file name was specified */
	if (!original_file_name) {
		result = MAYBE_ERROR_LOGGER_FILE_INVALID_FILE_NAME;
		goto l_cleanup;
	}

	state = (internal_state_t*)malloc(sizeof(internal_state_t));
	if (!state) {
		result = MAYBE_ERROR_LOGGER_FILE_ERROR;
		goto l_cleanup;
	}

	/* Create a copy of the file name */
	original_file_name_size = strlen((const char*)original_file_name);
	state->file_name = (uint8_t*)malloc(original_file_name_size + 1);
	if (!state->file_name) {
		result = MAYBE_ERROR_LOGGER_FILE_ERROR;
		goto l_cleanup;
	}

	memcpy(state->file_name, original_file_name, original_file_name_size);
	state->file_name[original_file_name_size] = '\0';

	state->file_pointer = fopen((const char*)original_file_name, "w");
	if (!state->file_pointer) {
		result = MAYBE_ERROR_LOGGER_FILE_ERROR;
		goto l_cleanup;
	}

	logger->platform_state = (void*)state;
	
	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_logger_platforms_file_write(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	uint8_t* data,
	uint32_t size,
	void* params
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	internal_state_t* state = (internal_state_t*)logger->platform_state;

	/* Write the data to the log file */
	if (size != fwrite((const void*)data, 1, size, state->file_pointer)) {
		result = MAYBE_ERROR_LOGGER_FILE_WRITE_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

void maybe_logger_platforms_file_free(
	maybe_logger_t* logger
) {
	internal_state_t* state = (internal_state_t*)logger->platform_state;

	if (state) {
		if (state->file_name) {
			free(state->file_name);
		}

		if (state->file_pointer) {
			fclose(state->file_pointer);
		}

		free(state);
	}
}
