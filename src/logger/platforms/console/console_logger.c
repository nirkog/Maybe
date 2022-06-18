#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/error.h"

#include "console_logger.h"
#include "console_logger_internal.h"

log_level_color_t log_level_colors[] = {
	[MAYBE_LOGGER_LOG_LEVEL_DEBUG] 		= { NULL, "34" },	
	[MAYBE_LOGGER_LOG_LEVEL_INFO] 		= { NULL, "33" },	
	[MAYBE_LOGGER_LOG_LEVEL_WARNING] 	= { NULL, "154" },	
	[MAYBE_LOGGER_LOG_LEVEL_ERROR] 		= { "188", "160" },	
};

maybe_error_t maybe_logger_platforms_console_init(
	maybe_logger_t* logger,
	void* params
) {
	return MAYBE_ERROR_SUCCESS;
}

maybe_error_t maybe_logger_platforms_console_write(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	uint8_t* data,
	uint32_t size,
	void* params
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	log_level_color_t colors = log_level_colors[log_level];
	uint8_t* color_sequence = NULL;

	/* Generate the color escape sequence */
	if (!get_color_escape_sequence(colors.background, colors.foreground, &color_sequence)) {
		result = MAYBE_ERROR_LOGGER_CONSOLE_ERROR;
		goto l_cleanup;
	}

	printf("%s%s%s", color_sequence, data, RESET_COLORS_ESCAPE_SEQUENCE);

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	if (color_sequence) {
		free(color_sequence);
	}

	return result;
}

bool get_color_escape_sequence(
	const char* background_color,
	const char* foreground_color,
	uint8_t** escape_sequence
) {
	bool result = false;
	uint8_t* _escape_sequence = NULL;
	uint8_t* escape_sequence_pointer = NULL;
	uint8_t size = EMPTY_BACKGROUND_FOREGROUND_COLOR_ESCAPE_SEQUENCE_SIZE;

	/* Calculate the sequence size */
	if (foreground_color) {
		size += strlen(foreground_color);
	}

	if (background_color) {
		size += strlen(foreground_color);
	}

	/* Allocate memory for the sequence */
	_escape_sequence = malloc(size + 1);	
	if (!_escape_sequence) {
		goto l_cleanup;
	}

	escape_sequence_pointer = _escape_sequence;
	escape_sequence_pointer[size] = '\0';

	/* Copy the generic escape sequence prefix */
	memcpy(escape_sequence_pointer, ESCAPE_SEQUENCE_PREFIX, strlen(ESCAPE_SEQUENCE_PREFIX));
	escape_sequence_pointer += strlen(ESCAPE_SEQUENCE_PREFIX);
	
	if (foreground_color) {
		/* Copy the foreground prefix */
		memcpy(escape_sequence_pointer, FOREGROUND_COLOR_ESCAPE_SEQUENCE, strlen(FOREGROUND_COLOR_ESCAPE_SEQUENCE));
		escape_sequence_pointer += strlen(FOREGROUND_COLOR_ESCAPE_SEQUENCE);

		/* Copy the foreground color */
		memcpy(escape_sequence_pointer, foreground_color, strlen(foreground_color));
		escape_sequence_pointer += strlen(foreground_color);
	}

	if (background_color) {
		if (foreground_color) {
			*escape_sequence_pointer = ';';
			escape_sequence_pointer++;
		}

		/* Copy the background prefix */
		memcpy(escape_sequence_pointer, BACKGROUND_COLOR_ESCAPE_SEQUENCE, strlen(BACKGROUND_COLOR_ESCAPE_SEQUENCE));
		escape_sequence_pointer += strlen(BACKGROUND_COLOR_ESCAPE_SEQUENCE);

		/* Copy the background color */
		memcpy(escape_sequence_pointer, background_color, strlen(background_color));
		escape_sequence_pointer += strlen(background_color);
	}

	*escape_sequence_pointer = 'm';
	escape_sequence_pointer++;

	*escape_sequence = _escape_sequence;

	result = true;
l_cleanup:
	return result;
}

void maybe_logger_platforms_console_free(
	maybe_logger_t* logger
) {
}
