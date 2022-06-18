#include <stddef.h>
#include <stdio.h>

#include "logger/logger.h"

extern int application_init();

int main() {
	maybe_logger_t logger;

	maybe_logger_init(&logger, MAYBE_LOGGER_LOG_LEVEL_DEBUG, MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE, NULL);

	MAYBE_LOGGER_DEBUG_WRITE(&logger, "HLASLAasjlasjlasjS");
	MAYBE_LOGGER_ERROR_WRITE(&logger, "{1f}, {0i}, {2l}a", 12, 11.23f, 12182182012128l);
	MAYBE_LOGGER_WARNING_WRITE(&logger, "{1f}, {0i}, {2l}a", 12, 11.23f, 12182182012128l);

	application_init();

	return 0;
}
