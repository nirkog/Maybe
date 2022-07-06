#pragma once

#include <stdint.h>

#include "common/common.h"
#include "common/vector/vector.h"
#include "ecs/archetype.h"

/* @TODO Add maps to link between component ID and component ID index */

/* @brief A prototype for a system function */
typedef void (*maybe_system_function_t)(void* system);

/* @brief The needed info for a system about an archetype it needs to iterate */
typedef struct {
	maybe_archetype_t* archetype;
	uint32_t* component_indices;
} maybe_system_archetype_info_t;

/* @brief An iterator used in systems to iterate through its requested components */
typedef struct {
	uint32_t component_id;
	uint32_t component_id_index;
	uint32_t current_archetype_index;
	maybe_vector_t* current_component_vector;
	uint32_t current_component_index;
	void* current_component_pointer; 
} maybe_system_component_iterator_t;

/* @brief The state of a system */
typedef struct {
	maybe_system_function_t function;
	MAYBE_VECTOR(maybe_system_archetype_info_t) archetypes;
	uint32_t* component_ids;
	uint32_t component_count;
	maybe_system_component_iterator_t* iterators;
} maybe_system_t;

/*
 * @brief Initialize a system using a va_list for the component IDs
 *
 * @param system A pointer to the system
 * @param function The actual system function that will be run on update
 * @param component_count The amount of components requested by the system
 * @param components A list of the component IDs used by the system
 * */
maybe_error_t maybe_system_init_va_list(
	maybe_system_t* system,
	maybe_system_function_t function,
	uint32_t component_count,
	va_list components
);

/*
 * @brief Initialize a system
 *
 * @param system A pointer to the system
 * @param function The actual system function that will be run on update
 * @param component_count The amount of components requested by the system
 * @param components A list of the component IDs used by the system
 * */
maybe_error_t maybe_system_init(
	maybe_system_t* system,
	maybe_system_function_t function,
	uint32_t component_count,
	...
);

/*
 * @brief Try to add an archetype to the system for iteration
 *
 * @param system A pointer to the system
 * @param archetype The archetype to add 
 * */
maybe_error_t maybe_system_add_archetype(
	maybe_system_t* system,
	maybe_archetype_t* archetype
);

/*
 * @brief Initialize a component iterator used by the system function
 *
 * @param system A pointer to the system
 * @param component_id The ID of the component type the iterator should iterate
 * @param iterator The new iterator
 * */
maybe_error_t maybe_system_init_component_iterator(
	maybe_system_t* system,
	uint32_t component_id,
	maybe_system_component_iterator_t* iterator
);

/*
 *  @brief Get the next component in a component iterator
 *
 *  @param system A pointer to the system
 *  @param iterator A pointer to the iterator
 * */
maybe_error_t maybe_system_component_iterator_get_next_component(
	maybe_system_t* system,
	maybe_system_component_iterator_t* iterator
);

/*
 * @brief Free a system's resources
 *
 * @param system A pointer to the system
 * */
maybe_error_t maybe_system_free(
	maybe_system_t* system
);
