#include <stdint.h>
#include <time.h>

#include "time.h"

uint64_t init_time_ns = 0;
uint64_t init_time_seconds = 0;

void maybe_time_init() {
	struct timespec ts;

    timespec_get(&ts, TIME_UTC);

	init_time_ns = (uint64_t)ts.tv_nsec;
	init_time_seconds = (uint64_t)ts.tv_sec;
}

uint64_t maybe_time_get_time_since_epoch_ns() {
	struct timespec ts;

    timespec_get(&ts, TIME_UTC);

	return (uint64_t)ts.tv_nsec;
}

uint64_t maybe_time_get_time_since_epoch_seconds() {
	struct timespec ts;

    timespec_get(&ts, TIME_UTC);

	return (uint64_t)ts.tv_sec;
}

uint64_t maybe_time_get_time_since_init_ns() {
	return maybe_time_get_time_since_epoch_ns() - init_time_ns;
}

uint64_t maybe_time_get_time_since_init_seconds() {
	return maybe_time_get_time_since_epoch_seconds() - init_time_seconds;
}

uint64_t maybe_time_get_time_since_init_minutes() {
	return (maybe_time_get_time_since_epoch_seconds() - init_time_seconds) / 60;
}
