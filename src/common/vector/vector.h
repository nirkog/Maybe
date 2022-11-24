#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/error.h"

#define MAYBE_VECTOR_DEAULT_INITIAL_CAPCITY (10)

typedef struct {
	void* elements;
	uint32_t element_size;
	uint32_t length;
	uint32_t capacity;
} maybe_vector_t;

/*
 * @brief Initialize a vector
 *
 * @param vector A pointer to the new vector
 * @param element_size The size of an element in the vector
 * @param initial_capacity The initial capcity of the vector, if 0 
 * 		  the initial capacity will be set to a default value
 * */
maybe_error_t maybe_vector_init(
	maybe_vector_t* vector,
	uint32_t element_size,
	uint32_t initial_capacity
);

/* @brief Push an element to the end of a vector
 *
 * @param vector A pointer to the vector
 * @param element The new element's data, if NULL the element will not be initialized
 * */
maybe_error_t maybe_vector_push(
	maybe_vector_t* vector,
	void* element	
);

/*
 * @brief Remove an element at a specified index from a vector
 *
 * @param vector A pointer to the vector
 * @param index The element's index
 * */
maybe_error_t maybe_vector_remove(
	maybe_vector_t* vector,
	uint32_t index
);

/*
 * @brief Free a vector's resources
 *
 * @param vector The vector to be freed
 * */
maybe_error_t maybe_vector_free(
	maybe_vector_t* vector
);

/*
 * @brief Resize a vector, allocate more memory if needed
 *
 * @param vector A vector
 * @param length The new length in elements
 * */
maybe_error_t maybe_vector_resize(
	maybe_vector_t* vector,
	uint32_t length
);

#define MAYBE_VECTOR_ELEMENT(vector, type, index) (((type*)(vector).elements)[index])
#define MAYBE_VECTOR_PTR_ELEMENT(vector, type, index) (((type*)(vector)->elements)[index])

#define MAYBE_VECTOR_DATA(vector, type) ((type*)((vector).elements))
#define MAYBE_VECTOR_PTR_DATA(vector, type) ((type*)((vector)->elements))

#define MAYBE_VECTOR_ELEMENT_VOID_PTR(vector, index) ((void*)(((uint8_t*)(vector).elements + (index * (vector).element_size))))
#define MAYBE_VECTOR_PTR_ELEMENT_VOID_PTR(vector, index) ((void*)(((uint8_t*)(vector)->elements + (index * (vector)->element_size))))

#define MAYBE_VECTOR(type) maybe_vector_t
#define MAYBE_VECTOR_PTR(type) maybe_vector_t*

#define MAYBE_VECTOR_EMPTY(vector) ((vector).length == 0)
#define MAYBE_VECTOR_PTR_EMPTY(vector) ((vector)->length == 0)

#define MAYBE_VECTOR_INIT(vector, type) (maybe_vector_init(&vector, sizeof(type), 0))
#define MAYBE_VECTOR_PTR_INIT(vector, type) (maybe_vector_init(&vector, sizeof(type), 0))
