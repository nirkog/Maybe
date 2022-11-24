#pragma once

#include <stdint.h>

/*
 * @brief Clamp a value between a maximum and a minimum
 *
 * @param value The value to be clamped
 * @param min The minimum value
 * @param max The maximum value
 * */
uint32_t maybe_utils_clamp_u32(
	uint32_t value,
	uint32_t min,
	uint32_t max
);
