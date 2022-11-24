#include <stdio.h>

#include "io.h"
#include "io_internal.h"

maybe_error_t maybe_utils_io_read_file(
	const char* path,
	uint8_t** buffer,
	uint32_t* size
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	FILE* file_pointer = NULL;
	long file_size = 0;
	uint8_t* inner_buffer = NULL;

	if ((NULL == path) || (NULL == buffer)) {
		result = MAYBE_ERROR_UTILS_IO_NULL_PARAM;
		goto l_cleanup;
	}
	
	/* Open file for reading */
	file_pointer = fopen(path, FILE_OPEN_MODIFIER);
	if (NULL == file_pointer) {
		result = MAYBE_ERROR_UTILS_IO_COULD_NOT_OPEN_FILE;
		goto l_cleanup;
	}

	/* Get file size */
	if (FSEEK_FAIL_RETURN_VALUE == fseek(file_pointer, 0, SEEK_END)) {
		result = MAYBE_ERROR_UTILS_IO_FSEEK_FAILED;
		goto l_cleanup;
	}

	file_size = ftell(file_pointer);
	if (FTELL_FAIL_RETURN_VALUE == file_size) {
		result = MAYBE_ERROR_UTILS_IO_FTELL_FAILED;
		goto l_cleanup;
	}

	if (FSEEK_FAIL_RETURN_VALUE == fseek(file_pointer, 0, SEEK_SET)) {
		result = MAYBE_ERROR_UTILS_IO_FSEEK_FAILED;
		goto l_cleanup;
	}

	inner_buffer = MALLOC_T(uint8_t, file_size);
	if (NULL == inner_buffer) {
		result = MAYBE_ERROR_UTILS_IO_MALLOC_FAILED;
		goto l_cleanup;
	}
	
	/* Read file */
	if (file_size != fread(inner_buffer, 1, file_size, file_pointer)) {
		result = MAYBE_ERROR_UTILS_IO_COULD_NOT_READ_FILE;
		goto l_cleanup;
	}

	/* Save buffer and its size */
	*buffer = inner_buffer;
	*size = file_size;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	if (IS_FAILURE(result) && (NULL != inner_buffer)) {
		free(inner_buffer);
	}

	return result;
}
