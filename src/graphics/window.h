#include <stdint.h>
#include <stdbool.h>

#include <GLFW/glfw3.h>

#include "common/common.h"

typedef struct {
	GLFWwindow* window;
	const char* title;
	uint32_t width;
	uint32_t height;
} maybe_window_t;

/*
 * @brief Initialize a window
 *
 * @param window A pointer to the window
 * */
maybe_error_t maybe_window_init(
	maybe_window_t* window,
	const char* title,
	uint32_t width,
	uint32_t height
);

/*
 * @brief Check if a windows should be closed (probably due to an IO event)
 *
 * @param window The window
 * */
bool maybe_window_should_be_closed(
	maybe_window_t* window
);

/*
 * @brief Free a window's resources
 *
 * @param window A pointer to the window
 * */
maybe_error_t maybe_window_free(
	maybe_window_t* window
);
