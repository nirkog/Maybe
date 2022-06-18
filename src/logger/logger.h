#pragma once

#include <stdint.h>

#include "common/error.h"

/*
 * @brief The platform type a logger will output to
 * */
typedef enum {
	MAYBE_LOGGER_PLATFORM_TYPE_FILE = 0,
	MAYBE_LOGGER_PLATFORM_TYPE_CONSOLE,
	MAYBE_LOGGER_PLATFORM_TYPE_COUNT
} maybe_logger_platform_type_t;

/*
 * @brief Logger log levels
 * */
typedef enum {
	MAYBE_LOGGER_LOG_LEVEL_DEBUG,
	MAYBE_LOGGER_LOG_LEVEL_INFO,
	MAYBE_LOGGER_LOG_LEVEL_WARNING,
	MAYBE_LOGGER_LOG_LEVEL_ERROR,
} maybe_logger_log_level_t;

/*
 * @brief The main logger struct. In order to use a logger, create an instance of this struct
 * 		  and initialize it using maybe_logger_init
 * */	
typedef struct {
	maybe_logger_platform_type_t platform;
	maybe_logger_log_level_t log_level;
	void* platform_state;
} maybe_logger_t;

/*
 * @brief An interface for a logging platform
 * */
typedef struct {
	maybe_error_t (*init)(maybe_logger_t* logger, void* params);
	maybe_error_t (*write)(maybe_logger_t* logger, uint8_t* data, uint32_t size, void* params);
} maybe_logger_platform_t;

/*
 * @brief Initialze a logger for future use
 * 
 * @param logger The logger instance to be initialized
 * @param log_level The logger's log level
 * @param platform The logger's output platform
 * @param params A pointer to the specific logging platform's parameter struct, can be null
 * 				 if no paramteres are required
 * */
maybe_error_t maybe_logger_init(
	maybe_logger_t* logger,
	maybe_logger_log_level_t log_level,
	maybe_logger_platform_type_t platform,
	void* params
);

/*
 * @brief Write a message using a logger
 *
 * @param logger The logger
 * @param params Platform specific write parameters
 * @param format The message format
 *
 * @note The rest of the arguments are dependant of the format
 * 
 * @note The logger currently only supports up to 10 arguments and 30 argument references
 * */
maybe_error_t maybe_logger_write(
	maybe_logger_t* logger,
	void* params,
	const char* format,
	...
);
