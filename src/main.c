#include <stddef.h>
#include <stdio.h>

#include "logger/logger.h"
#include "time/time.h"

extern int application_init(maybe_logger_t* logger);

int main() {
	maybe_logger_t logger;

	maybe_time_init();

	maybe_logger_init(&logger, MAYBE_LOGGER_LOG_LEVEL_DEBUG, MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE, NULL);

	MAYBE_LOGGER_DEBUG_WRITE(&logger, "Initializing engine");

	application_init(&logger);

	maybe_logger_free(&logger);

	return 0;
}
