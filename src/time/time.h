#pragma once

/*
 * @note Most functions in this module do not throw, but some will
 * 		 return unpredictable results it the module is not initialized
 * @TODO consider if this is the right choice
 * */

/*
 * @brief Initialize the time module
 * */
void maybe_time_init(void);

/*
 * @brief Get the current time since epoch in nanoseconds
 * */
uint64_t maybe_time_get_time_since_epoch_ns(void);

/*
 * @brief Get the current time since epoch in seconds
 * */
uint64_t maybe_time_get_time_since_epoch_seconds(void);

/*
 * @brief Get the current time since init in nanoseconds
 * */
uint64_t maybe_time_get_time_since_init_ns(void);

/*
 * @brief Get the current time since init in seconds
 * */
uint64_t maybe_time_get_time_since_init_seconds(void);

/*
 * @brief Get the current time since init in minutes (rounded down)
 * */
uint64_t maybe_time_get_time_since_init_minutes(void);
