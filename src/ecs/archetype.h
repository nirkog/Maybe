#pragma once

#include <stdint.h>

#include "common/error.h"
#include "common/vector/vector.h"

/* @TODO Consider grouping together each entity's components together to optimize iteration */
typedef struct {
	uint32_t component_types_count;
	MAYBE_VECTOR(maybe_vector_t) components; /* @TODO Maybe change this to a map of <component_id, component_storage> */
	MAYBE_VECTOR(uint32_t) component_ids;
} maybe_archetype_t;

/*
 * @brief Initialize an archetype
 *
 * @param archetype The archetype
 * */
maybe_error_t maybe_archetype_init(
	maybe_archetype_t* archetype
);

/*
 * @brief Add a component type to an archetype
 *
 * @param archetype The archetype
 * @param component_id The id of the component to be added
 * @param component_size The size of an instance of the component type
 * */
maybe_error_t maybe_archetype_add_component_type(
	maybe_archetype_t* archetype,
	uint32_t component_id,
	uint32_t component_size
);

/*
 * @brief Free an archetype's resources
 *
 * @param archetype The archetype
 * */
maybe_error_t maybe_archetype_free(
	maybe_archetype_t* archetype
);
