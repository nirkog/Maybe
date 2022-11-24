#include <stddef.h>
#include <stdio.h>

#include "common/common.h"
#include "logger/logger.h"
#include "time/time.h"
#include "graphics/window.h"
#include "graphics/graphics.h"
#include "utils/io.h"

extern int application_init(void);

int main() {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_window_t window;

	maybe_time_init();

	/* Initialize logger */
	result = MAYBE_LOGGER_INIT(MAYBE_LOGGER_LOG_LEVEL_DEBUG, MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE, NULL);
	if (IS_FAILURE(result)) {
		MAYBE_ERROR_LOG("Maybe fatal error: failed to initialize logger");
		goto l_cleanup;
	}

	/* Initialize graphics */
	MAYBE_INFO_LOG("Initializing graphics");
	result = maybe_graphics_init();
	if (IS_FAILURE(result)) {
		MAYBE_ERROR_LOG("Failed to initialize graphics, error code {0u}", (uint32_t)result);
		goto l_cleanup;
	}

	application_init();

	while (!maybe_graphics_should_shutdown()) {
		maybe_graphics_poll_events();
        maybe_graphics_draw_frame();
	}

	MAYBE_INFO_LOG("Shutting down");

l_cleanup:
	MAYBE_LOGGER_FREE();
	maybe_graphics_terminate();

	return result;
}
