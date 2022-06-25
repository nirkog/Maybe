#include <stdio.h>

#include <logger/logger.h>
#include <common/map/map.h>
#include <common/common.h>
#include <common/error.h>

typedef struct {
	uint32_t math;
	uint32_t english;
} grade_t;

int application_init() {
	maybe_map_t map;
	grade_t johx_grade = { 95, 80 };
	grade_t john_grade = { 64, 75 };
	grade_t johann_grade = { 82, 93 };
	grade_t* grade = NULL;
	maybe_error_t error;

	MAYBE_DEBUG_LOG("Initializing application");

	maybe_map_init(&map, sizeof(grade_t), 10, maybe_map_default_hash_function);

	maybe_map_set(&map, (void*)"John", 4, &john_grade);
	maybe_map_set(&map, (void*)"Johann", 6, &johann_grade);
	maybe_map_set(&map, (void*)"Johx", 4, &johx_grade);

	maybe_map_remove(&map, (void*)"John", 4);

	error = maybe_map_get(&map, (void*)"Johx", 4, (void**)&grade);
	if (grade) {
		MAYBE_DEBUG_LOG("Got math: {0i}, english: {1i}", grade->math, grade->english);
	}

	maybe_map_free(&map);

	return 0;
}
