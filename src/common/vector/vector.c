#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/error.h"
#include "common/common.h"
#include "logger/logger.h"

#include "vector.h"

maybe_error_t maybe_vector_init(
	maybe_vector_t* vector,
	uint32_t element_size,
	uint32_t initial_capacity
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == vector) {
		result = MAYBE_ERROR_VECTOR_NULL_PARAM;
		goto l_cleanup;
	}

	/* Set a default initial capcity if it 0 */
	if (initial_capacity == 0) {
		initial_capacity = MAYBE_VECTOR_DEAULT_INITIAL_CAPCITY;
	}

	/* Initialize the vector */
	vector->element_size = element_size;
	vector->length = 0;
	vector->capacity = initial_capacity;
	vector->elements = malloc(initial_capacity * element_size);
	if (NULL == vector->elements) {
		result = MAYBE_ERROR_VECTOR_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vector_push(
	maybe_vector_t* vector,
	void* element	
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == vector) {
		result = MAYBE_ERROR_VECTOR_NULL_PARAM;
		goto l_cleanup;
	}

	/* Allocate more memory if needed */
	if (vector->length == vector->capacity) {
		vector->capacity *= 2;
		vector->elements = realloc(vector->elements, vector->capacity * vector->element_size);
		if (NULL == vector->elements) {
			result = MAYBE_ERROR_VECTOR_ALLOCATION_FAILED;
			goto l_cleanup;
		}
	}

	/* Initialize element */
	if (element) {
		memcpy(
			(void*)((uint8_t*)(vector->elements + (vector->length * vector->element_size))),
			element,
			vector->element_size
		);
	}

	vector->length++;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vector_remove(
	maybe_vector_t* vector,
	uint32_t index
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == vector) {
		result = MAYBE_ERROR_VECTOR_NULL_PARAM;
		goto l_cleanup;
	}

	/* Validate index */
	if (index >= vector->length) {
		result = MAYBE_ERROR_VECTOR_INDEX_OUT_OF_RANGE;
		goto l_cleanup;
	}
	
	/* Move all elements after the selected element back one spot */
	memcpy(
		(void*)((uint8_t*)(vector->elements + (index * vector->element_size))),
		(void*)((uint8_t*)(vector->elements + ((index + 1) * vector->element_size))),
		(vector->length - index - 1) * vector->element_size
	);

	vector->length--;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vector_free(
	maybe_vector_t* vector
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == vector) {
		result = MAYBE_ERROR_VECTOR_NULL_PARAM;
		goto l_cleanup;
	}

	if (vector->elements) {
		free(vector->elements);
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
