#include <windows.h>

#include "logger/logger.h"

#include "windows_console_logger.h"

WORD log_level_colors[] = {
	[MAYBE_LOGGER_LOG_LEVEL_DEBUG] 	 = 10,
	[MAYBE_LOGGER_LOG_LEVEL_INFO] 	 = 9,
	[MAYBE_LOGGER_LOG_LEVEL_WARNING] = 14,
	[MAYBE_LOGGER_LOG_LEVEL_ERROR] 	 = (15<<4) | 12,
};

extern maybe_error_t maybe_logger_platforms_console_init(
	maybe_logger_t* logger,
	void* params
) {
	return MAYBE_ERROR_SUCCESS;
}

extern maybe_error_t maybe_logger_platforms_console_write(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	uint8_t* data,
	uint32_t size,
	void* params
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    WORD saved_attributes;
	uint32_t i, last_index;

    GetConsoleScreenBufferInfo(console, &console_info);
    saved_attributes = console_info.wAttributes;

	for (i = 0, last_index = 0; i < size; i++) {
		if (data[i] == '\n') {
			data[i] = '\0';
			SetConsoleTextAttribute(console, log_level_colors[log_level]);
			printf("%s", (char*)&data[last_index]);
			last_index = i + 1;

			SetConsoleTextAttribute(console, saved_attributes);
			printf("\n");
		}
	}

	if (last_index != size) {
		SetConsoleTextAttribute(console, log_level_colors[log_level]);
		printf("%s", (char*)&data[last_index]);

		SetConsoleTextAttribute(console, saved_attributes);
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

extern void maybe_logger_platforms_console_free(
	maybe_logger_t* logger
) {
}
