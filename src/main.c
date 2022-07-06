#include <stddef.h>
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "logger/logger.h"
#include "time/time.h"
#include "common/map/map.h"

extern int application_init(void);

/* @TODO Create a CHECK_ERROR macro for calling a function and checking for error in return value */

int main() {
	maybe_time_init();

	MAYBE_LOGGER_INIT(MAYBE_LOGGER_LOG_LEVEL_DEBUG, MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE, NULL);

	MAYBE_DEBUG_LOG("Initializing engine");

	/*
	if (!glfwInit()) {
		MAYBE_LOGGER_ERROR_WRITE(&logger, "GLFW Failed to init");
		goto l_cleanup;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	if (!window) {
		MAYBE_LOGGER_ERROR_WRITE(&logger, "Failed to create window");
		goto l_cleanup;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	*/


	application_init();

l_cleanup:
	MAYBE_LOGGER_FREE();

	glfwTerminate();

	return 0;
}
