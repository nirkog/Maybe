#include <stddef.h>
#include <stdio.h>

#include "logger/logger.h"

extern int application_init();

int main() {
	maybe_logger_t logger;

	maybe_logger_init(&logger, MAYBE_LOGGER_LOG_LEVEL_DEBUG, MAYBE_LOGGER_PLATFORM_TYPE_FILE, "log");

	MAYBE_LOGGER_DEBUG_WRITE(&logger, "HLASLAasjlasjlasjS");

	application_init();

	maybe_logger_free(&logger);

	return 0;
}
