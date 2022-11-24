#include <stdint.h>

#include <GLFW/glfw3.h>

#include "window.h"

maybe_error_t maybe_window_init(
	maybe_window_t* window,
	const char* title,
	uint32_t width,
	uint32_t height
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == window) {
		result = MAYBE_ERROR_WINDOW_NULL_PARAM;
		goto l_cleanup;
	}

	window->title = title;
	window->width = width;
	window->height = height;

	/* No OpenGL context and no resizing (for now) */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	/* Create a window */
	window->window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window->window) {
		result = MAYBE_ERROR_WINDOW_WINDOW_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

bool maybe_window_should_be_closed(
	maybe_window_t* window
) {
	return glfwWindowShouldClose(window->window);
}

maybe_error_t maybe_window_free(
	maybe_window_t* window
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == window) {
		result = MAYBE_ERROR_WINDOW_NULL_PARAM;
		goto l_cleanup;
	}
	
	glfwDestroyWindow(window->window);

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
