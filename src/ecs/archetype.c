#include <stdint.h>
#include <stddef.h>

#include "common/error.h"
#include "common/common.h"
#include "common/vector/vector.h"

#include "archetype.h"

maybe_error_t maybe_archetype_init(
	maybe_archetype_t* archetype
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == archetype) {
		result = MAYBE_ERROR_ARCHETYPE_NULL_PARAM;
		goto l_cleanup;
	}

	/* Initialize archetype */
	archetype->component_types_count = 0;
	result = maybe_vector_init(&archetype->components, sizeof(maybe_vector_t), 0);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}
	result = maybe_vector_init(&archetype->component_ids, sizeof(uint32_t), 0);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_archetype_add_component_type(
	maybe_archetype_t* archetype,
	uint32_t component_id,
	uint32_t component_size
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_vector_t component_vector;

	if (NULL == archetype) {
		result = MAYBE_ERROR_ARCHETYPE_NULL_PARAM;
		goto l_cleanup;
	}

	/* Add component ID */
	result = maybe_vector_push(&archetype->component_ids, &component_id);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	/* Create a vector to store the components */
	result = maybe_vector_init(&component_vector, component_size, 0);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	/* Add the component storage to the components vector */
	result = maybe_vector_push(&archetype->components, &component_vector);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	archetype->component_types_count++;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_archetype_free(
	maybe_archetype_t* archetype
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_error_t free_result;
	uint32_t i;

	if (NULL == archetype) {
		result = MAYBE_ERROR_ARCHETYPE_NULL_PARAM;
		goto l_cleanup;
	}

	/* @TODO Who's responsible for freeing the components resources */
	/* Iterate over components and free them */
	for (i = 0; i < archetype->component_types_count; i++) {
		free_result = maybe_vector_free(&MAYBE_VECTOR_ELEMENT(archetype->components, maybe_vector_t, i));
		if (IS_FAILURE(free_result)) {
			result = free_result;
		}
	}
	
	free_result = maybe_vector_free(&archetype->components);
	if (IS_FAILURE(free_result)) {
		result = free_result;
	}

	free_result = maybe_vector_free(&archetype->component_ids);
	if (IS_FAILURE(free_result)) {
		result = free_result;
	}

	/* If any free operation failed, return an error */
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
