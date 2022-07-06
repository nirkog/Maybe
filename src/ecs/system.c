#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common/common.h"
#include "common/vector/vector.h"

#include "system.h"

maybe_error_t maybe_system_init_va_list(
	maybe_system_t* system,
	maybe_system_function_t function,
	uint32_t component_count,
	va_list components
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t i, component_id;

	if ((NULL == system) || (NULL == function)) {
		result = MAYBE_ERROR_SYSTEM_NULL_PARAM;
		goto l_cleanup;
	}

	/* Initialize struct with parameters */
	system->function = function;
	system->component_count = component_count;
	system->component_ids = (uint32_t*)malloc(component_count * sizeof(uint32_t));
	if (NULL == system->component_ids) {
		result = MAYBE_ERROR_SYSTEM_ALLOCATION_FAILED;
		goto l_cleanup;
	}
	system->iterators = MALLOC_T(maybe_system_component_iterator_t, component_count);
	if (NULL == system->iterators) {
		result = MAYBE_ERROR_SYSTEM_ALLOCATION_FAILED;
		goto l_cleanup;
	}
	if (IS_FAILURE(maybe_vector_init(&system->archetypes, sizeof(maybe_system_archetype_info_t), 0))) {
		goto l_cleanup;
	}	
	
	/* Initialize component ids */
	for (i = 0; i < component_count; i++) {
		component_id = va_arg(components, uint32_t);
		system->component_ids[i] = component_id;
		system->iterators[i].component_id = component_id;
		system->iterators[i].component_id_index = i;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_system_init(
	maybe_system_t* system,
	maybe_system_function_t function,
	uint32_t component_count,
	...
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	va_list components;

	va_start(components, component_count);

	maybe_system_init_va_list(system, function, component_count, components);

	va_end(components);

	return result;
}

maybe_error_t maybe_system_add_archetype(
	maybe_system_t* system,
	maybe_archetype_t* archetype
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_system_archetype_info_t info = { NULL };
	uint32_t* component_id_indices = NULL; /* @TODO Maybe optimize this with a global */
	uint32_t i, j;
	bool found_component = false;

	if ((NULL == system) || (NULL == archetype)) {
		result = MAYBE_ERROR_SYSTEM_NULL_PARAM;
		goto l_cleanup;
	}
	
	/* Verify that there are enough components in the archetype */
	if (archetype->component_types_count< system->component_count) {
		result = MAYBE_ERROR_SYSTEM_BAD_ARCHETYPE;
		goto l_cleanup;
	}

	/* Allocate memory for the indices of the component vectors in the archetype */
	component_id_indices = (uint32_t*)malloc(system->component_count * sizeof(uint32_t*));
	if (NULL == component_id_indices) {
		result = MAYBE_ERROR_SYSTEM_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	/* Make sure the archetype contains all the needed components, and find their indices */
	for (i = 0; i < system->component_count; i++) {
		found_component = false;
		for (j = 0; j < archetype->component_types_count; j++) {
			if (system->component_ids[i] == MAYBE_VECTOR_ELEMENT(archetype->component_ids, uint32_t, j)) {
				component_id_indices[i] = j;
				found_component = true;
				break;
			}
		}

		/* Did not found a component in the archetype */
		if (!found_component) {
			result = MAYBE_ERROR_SYSTEM_BAD_ARCHETYPE;
			goto l_cleanup;
		}
	}

	/* Save archetype info */
	info.archetype = archetype;
	info.component_indices = component_id_indices;

	/* Add archetype info to vector */
	if (IS_FAILURE(maybe_vector_push(&system->archetypes, &info))) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	if (IS_FAILURE(result)) {
		if (component_id_indices) {
			free(component_id_indices);
		}
	}

	return result;
}

maybe_error_t maybe_system_init_component_iterator(
	maybe_system_t* system,
	uint32_t component_id,
	maybe_system_component_iterator_t* iterator
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t i, component_id_index;
	maybe_system_archetype_info_t* archetype_info;
	maybe_archetype_t* archetype;

	if ((NULL == system) || (NULL == iterator)) {
		result = MAYBE_ERROR_SYSTEM_NULL_PARAM;
		goto l_cleanup;
	}

	/* Find the index of the component in the system's component IDs vector */
	iterator->component_id = component_id;
	for (i = 0; i < system->component_count; i++) {
		if (system->component_ids[i] == component_id) {
			component_id_index = i;
			iterator->component_id_index = i;
			break;
		}
	}

	iterator->current_archetype_index = 0;
	iterator->current_component_index = 0;

	if (system->archetypes.length == 0) {
		iterator->current_component_pointer = NULL;
	} else {
		archetype_info = &MAYBE_VECTOR_ELEMENT(system->archetypes, maybe_system_archetype_info_t, 0);
		archetype = archetype_info->archetype; 
		iterator->current_component_vector = &MAYBE_VECTOR_ELEMENT(archetype->components, maybe_vector_t, archetype_info->component_indices[component_id_index]);
		iterator->current_component_pointer = MAYBE_VECTOR_PTR_ELEMENT_VOID_PTR(iterator->current_component_vector, 0);
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_system_component_iterator_get_next_component(
	maybe_system_t* system,
	maybe_system_component_iterator_t* iterator
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_system_archetype_info_t* archetype_info;
	maybe_archetype_t* archetype;
	
	if ((NULL == system) || (NULL == iterator)) {
		result = MAYBE_ERROR_SYSTEM_NULL_PARAM;
		goto l_cleanup;
	}

	iterator->current_component_index++;

	/* If the next component is in the same archetype, Advance the component pointer. Else, reset the component index 
	 * and point to the next archetype. If the archetypes are over, set the component pointer to NULL, and return an error */
	if (iterator->current_component_index < iterator->current_component_vector->length) {
		iterator->current_component_pointer = MAYBE_VECTOR_PTR_ELEMENT_VOID_PTR(iterator->current_component_vector, iterator->current_component_index);
	} else {
		iterator->current_archetype_index++;
		iterator->current_component_index = 0;

		/* Last component reached */
		if (iterator->current_archetype_index >= system->archetypes.length) {
			iterator->current_component_pointer = NULL;
			result = MAYBE_ERROR_SYSTEM_COMPONENT_ITERATOR_LAST_COMPONENT_REACHED;
			goto l_cleanup;
		}

		/* Start new archetype */
		archetype_info = &MAYBE_VECTOR_ELEMENT(system->archetypes, maybe_system_archetype_info_t, iterator->current_archetype_index);
		archetype = archetype_info->archetype; 
		iterator->current_component_vector = &MAYBE_VECTOR_ELEMENT(archetype->components, maybe_vector_t, archetype_info->component_indices[iterator->component_id_index]);
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_system_free(
	maybe_system_t* system
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t i;

	if (NULL == system) {
		result = MAYBE_ERROR_SYSTEM_NULL_PARAM;
		goto l_cleanup;
	}

	if (system->component_ids) {
		free(system->component_ids);
	}

	if (system->iterators) {
		free(system->iterators);
	}

	for (i = 0; i < system->component_count; i++) {
		if (MAYBE_VECTOR_ELEMENT(system->archetypes, maybe_system_archetype_info_t, i).component_indices) {
			free(MAYBE_VECTOR_ELEMENT(system->archetypes, maybe_system_archetype_info_t, i).component_indices);
		}
	}

	maybe_vector_free(&system->archetypes);

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
