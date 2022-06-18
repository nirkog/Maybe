#include <stdio.h>

#include <logger/logger.h>

int application_init(maybe_logger_t* logger) {
	MAYBE_LOGGER_DEBUG_WRITE(logger, "Initializing application");

	return 0;
}
