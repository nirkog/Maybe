#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common/error.h"
#include "common/common.h"
#include "common/map/map.h"

#include "ecs.h"
#include "ecs_internal.h"

maybe_error_t maybe_world_init(
	maybe_world_t* world
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == world) {
		result = MAYBE_ERROR_ECS_WORLD_NULL_PARAM;
		goto l_cleanup;
	}

	result = maybe_map_init(&world->entities, sizeof(maybe_world_record_t), 0, maybe_map_default_hash_function);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = maybe_vector_init(&world->archetypes, sizeof(maybe_archetype_t), 0);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = maybe_vector_init(&world->component_types, sizeof(maybe_component_type_t), 0);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = maybe_vector_init(&world->systems, sizeof(maybe_system_t), 0);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	world->next_entity_id = 0;
	world->next_component_id = 0;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_world_add_component_type(
	maybe_world_t* world,
	uint32_t component_size,
	uint32_t* component_id
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if ((NULL == world) || (NULL == component_id)) {
		result = MAYBE_ERROR_ECS_WORLD_NULL_PARAM;
		goto l_cleanup;
	}

	/* Add component type info to component types */
	result = maybe_vector_push(&world->component_types, (void*)&(maybe_component_type_t){ world->next_component_id, component_size });
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	*component_id = world->next_component_id;
	world->next_component_id++;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_world_add_entity(
	maybe_world_t* world,
	uint32_t component_count,
	maybe_entity_t* entity_id,
	...
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_error_t system_result = MAYBE_ERROR_UNINITIALIZED;
	va_list args;
	uint32_t i, component_id;
	uint32_t* component_ids = NULL;
	uint32_t* component_indices = NULL;
	maybe_vector_t* component_vector = NULL;
	maybe_archetype_t* archetype = NULL;

	if ((NULL == world) || (NULL == entity_id)) {
		result = MAYBE_ERROR_ECS_WORLD_NULL_PARAM;
		goto l_cleanup;
	}

	/* Allocate memory for the list of component ids */
	component_ids = (uint32_t*)malloc(component_count * sizeof(uint32_t));
	if (NULL == component_ids) {
		result = MAYBE_ERROR_ECS_WORLD_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	component_indices = (uint32_t*)malloc(component_count * sizeof(uint32_t));
	if (NULL == component_indices) {
		result = MAYBE_ERROR_ECS_WORLD_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	va_start(args, entity_id);

	/* Get all component ids */
	for (i = 0; i < component_count; i++) {
		component_id = va_arg(args, uint32_t);
		component_ids[i] = component_id;
	}	

	archetype = find_matching_archetype(world, component_ids, component_count, component_indices);

	/* If no mathing archetype was found, create a new one */
	if (!archetype) {
		result = maybe_vector_push(&world->archetypes, NULL);
		if (IS_FAILURE(result)) {
			goto l_cleanup;
		}

		/* Initialize the archetype */
		archetype = &MAYBE_VECTOR_ELEMENT(world->archetypes, maybe_archetype_t, world->archetypes.length - 1);
		result = maybe_archetype_init(archetype);
		if (IS_FAILURE(result)) {
			goto l_cleanup;
		}

		/* Add the entity's component types to the archetype */
		for (i = 0; i < component_count; i++) {
			component_indices[i] = i;
			result = maybe_archetype_add_component_type(
				archetype, 
				component_ids[i],
				MAYBE_VECTOR_ELEMENT(world->component_types, maybe_component_type_t, component_ids[i]).component_size
			);
			if (IS_FAILURE(result)) {
				goto l_cleanup;
			}
		}

		/* Try to add the new archetype to all systems */
		for (i = 0; i < world->systems.length; i++) {
			system_result = maybe_system_add_archetype(&MAYBE_VECTOR_ELEMENT(world->systems, maybe_system_t, i), archetype);
			if (IS_FAILURE(system_result)) {
				if (MAYBE_ERROR_SYSTEM_BAD_ARCHETYPE != system_result) {
					result = system_result;
					goto l_cleanup;
				}
			}
		}
	}

	/* Add the entity's components to the archetype */
	for (i = 0; i < component_count; i++) {
		component_vector = &MAYBE_VECTOR_ELEMENT(archetype->components, maybe_vector_t, component_indices[i]);

		result = maybe_vector_push(component_vector, NULL);
		if (IS_FAILURE(result)) {
			goto l_cleanup;
		}
	}

	world->next_entity_id++;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	va_end(args);

	if (component_ids) {
		free(component_ids);
	}

	if (component_indices) {
		free(component_indices);	
	}

	return result;
}

maybe_error_t maybe_world_register_system(
	maybe_world_t* world,
	maybe_system_function_t system_function,
	uint32_t component_count,
	...
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_system_t system;
	va_list components;

	if (NULL == world) {
		result = MAYBE_ERROR_ECS_WORLD_NULL_PARAM;
		goto l_cleanup;
	}

	va_start(components, component_count);

	/* Initialize the new system */
	if (IS_FAILURE(maybe_system_init_va_list(&system, system_function, component_count, components))) {
		goto l_cleanup;
	}

	/* Add system to vector */
	if (IS_FAILURE(maybe_vector_push(&world->systems, &system))) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	va_end(components);

	return result;
}

maybe_error_t maybe_world_update(
	maybe_world_t* world
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_system_t* system;
	uint32_t i;

	if (NULL == world) {
		result = MAYBE_ERROR_ECS_WORLD_NULL_PARAM;
		goto l_cleanup;
	}

	/* Call all systems */
	for (i = 0; i < world->systems.length; i++) {
		system = &MAYBE_VECTOR_ELEMENT(world->systems, maybe_system_t, i);
		system->function((void*)system);
	}	

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_world_free(
	maybe_world_t* world
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_error_t free_result;
	uint8_t i = 0;

	if (NULL == world) {
		result = MAYBE_ERROR_ECS_WORLD_NULL_PARAM;
		goto l_cleanup;
	}

	for (i = 0; i < world->archetypes.length; i++) {
		free_result = maybe_archetype_free(&MAYBE_VECTOR_ELEMENT(world->archetypes, maybe_archetype_t, i));
		if (IS_FAILURE(free_result)) {
			result = free_result;
		}
	}

	free_result = maybe_vector_free(&world->archetypes);
	if (IS_FAILURE(free_result)) {
		result = free_result;
	}

	free_result = maybe_vector_free(&world->component_types);
	if (IS_FAILURE(free_result)) {
		result = free_result;
	}

	for (i = 0; i < world->systems.length; i++) {
		free_result = maybe_system_free(&MAYBE_VECTOR_ELEMENT(world->systems, maybe_system_t, i));
		if (IS_FAILURE(free_result)) {
			result = free_result;
		}
	}

	free_result = maybe_vector_free(&world->systems);
	if (IS_FAILURE(free_result)) {
		result = free_result;
	}

	free_result = maybe_map_free(&world->entities);
	if (IS_FAILURE(free_result)) {
		result = free_result;
	}

	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_archetype_t* find_matching_archetype(
	maybe_world_t* world,
	uint32_t* component_ids,
	uint32_t component_count,
	uint32_t* component_indices
) {
	uint32_t i, j, k;
	bool found_archetype, found_component;
	maybe_archetype_t* archetype = NULL;

	/* @TODO Search smarter (Maybe by making the component IDs prime and multiplying them)  */
	for (i = 0; i < world->archetypes.length; i++) {
		archetype = &MAYBE_VECTOR_ELEMENT(world->archetypes, maybe_archetype_t, i);

		if (archetype->component_types_count != component_count) {
			continue;
		}

		found_archetype = true;
		for (j = 0; j < archetype->component_types_count; j++) {
			found_component = false;
			for (k = 0; k < component_count; k++) {
				if (component_ids[k] == MAYBE_VECTOR_ELEMENT(archetype->component_ids, uint32_t, j)) {
					component_indices[k] = j;
					found_component = true;
					break;
				}
			}

			if (!found_component) {
				found_archetype = false;
				break;
			}
		}

		if (found_archetype) {
			return archetype;
		}
	}
	
	return NULL;
}
