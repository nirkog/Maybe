#pragma once

#include <stdio.h>

/*
 * @brief An internal state for the file logger. Keeps track of the open file
 * 		  used for logging
 * */
typedef struct {
	uint8_t* file_name; /* @note This might be unnecessary */
	FILE* file_pointer;
} internal_state_t;
