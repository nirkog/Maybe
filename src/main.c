#include <stddef.h>
#include <stdio.h>

#include "logger/logger.h"

extern int application_init();

int main() {
	maybe_logger_t logger;

	printf("HELLO WORLD\n");

	maybe_logger_init(&logger, MAYBE_LOGGER_LOG_LEVEL_DEBUG, MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE, NULL);

	maybe_logger_write(&logger, NULL, "HELLO WORLD FROM THE LOGGER\n");

	application_init();

	return 0;
}
