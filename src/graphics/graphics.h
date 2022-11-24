#include <GLFW/glfw3.h>

#include "common/common.h"

/*
 * @brief Initialize the graphics module of maybe, should be called on engine initialization
 * */
maybe_error_t maybe_graphics_init(void);

/*
 * @brief Poll IO events
 * */
void maybe_graphics_poll_events(void);

/*
 * @brief Terminate the graphics module of maybe, should be called on engine exit
 * */
void maybe_graphics_terminate(void);

/*
 * @brief Should the engine shut down due to graphical reasons (window closing, etc.)
 * */
bool maybe_graphics_should_shutdown(void);

/*
 * @brief Draw a single frame
 * */
void maybe_graphics_draw_frame(void);
