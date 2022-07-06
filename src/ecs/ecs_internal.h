#include <stdint.h>

#include "common/common.h"
#include "archetype.h"

#include "ecs.h"

/*
 * @brief Find a matching archetype for a list of component types
 *
 * @param world The world
 * @param components An array of the component type IDs
 * @param component_count The amount of components
 * @param component_indices An array that will be filled with the indices of every component in the archetype
 * */
static maybe_archetype_t* find_matching_archetype(
	maybe_world_t* world,
	uint32_t* component_ids,
	uint32_t component_count,
	uint32_t* component_indices
);
