#pragma once

#include <stdbool.h>

#include "unix_console_logger.h"

#define EMPTY_BACKGROUND_FOREGROUND_COLOR_ESCAPE_SEQUENCE_SIZE (14)

#define ESCAPE_SEQUENCE_PREFIX ("\x1b[")

#define FOREGROUND_COLOR_ESCAPE_SEQUENCE ("38;5;")
#define BACKGROUND_COLOR_ESCAPE_SEQUENCE ("48;5;")

#define RESET_COLORS_ESCAPE_SEQUENCE ("\x1b[0m")

typedef struct {
	const char* background;
	const char* foreground;
} log_level_color_t;

typedef enum {
	CONSOLE_COLOR_TYPE_BACKGROUND,
	CONSOLE_COLOR_TYPE_FOREGROUND
} console_color_type_t;

/*
 * @brief Generate an escape sequence for a specific background and foreground colors
 * */
bool get_color_escape_sequence(
	const char* background_color,
	const char* foreground_color,
	uint8_t** escape_sequence
);
