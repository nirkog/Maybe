#pragma once

#include <stdint.h>

#include "common/map/map.h"
#include "common/vector/vector.h"
#include "entity.h"
#include "archetype.h"
#include "system.h"

/* @TODO Create a typedef for component_id */
/* @TODO Add a function that adds an entity and initializes its components */
/* @TODO Add some way to set an entity's components specifically */
/* @TODO Add documentation to the structs */

typedef struct {
	//maybe_archetype_t* archetype;
	uint32_t archetype_index;
	uint32_t row;	
} maybe_world_record_t;

typedef struct {
	uint32_t id;
	uint32_t component_size;
} maybe_component_type_t;

typedef struct {
	MAYBE_MAP(maybe_entity_id_t, maybe_world_record_t) entities;
	uint64_t next_entity_id;
	MAYBE_VECTOR(maybe_archetype_t) archetypes;
	MAYBE_VECTOR(maybe_component_type_t) component_types;
	uint32_t next_component_id;
	MAYBE_VECTOR(maybe_system_t) systems;
} maybe_world_t;

/*
 * @brief Initialize an ECS world
 *
 * @param world A pointer to the new ECS world
 * */
maybe_error_t maybe_world_init(
	maybe_world_t* world
);

/*
 * @brief Add a component type to an ECS world
 *
 * @param world A pointer to the ECS world
 * @param component_size The size of an instance of the componennt
 * @param component_id The resulting component type ID
 * */
maybe_error_t maybe_world_add_component_type(
	maybe_world_t* world,
	uint32_t component_size,
	uint32_t* component_id
);

/*
 * @brief Add an entity to an ECS world
 *
 * @param world A pointer to the world
 * @param component_count The number of components the new entity would have
 * @param entity_id The new entity's id
 *
 * @note The final parameters are the component IDs of the component the entity should have
 * */
maybe_error_t maybe_world_add_entity(
	maybe_world_t* world,
	uint32_t component_count,
	maybe_entity_t* entity_id,
	...
);

/*
 * @brief Remove an entity from an ECS world
 *
 * @param world A pointer to the world
 * @param entity_id The id of the entity to be removed
 * */
maybe_error_t maybe_world_remove_entity(
	maybe_world_t* world,
	maybe_entity_t entity_id
);

/*
 * @brief Register a system in a world.
 *
 * @param world A pointer to the world
 * @param system_function The system logic function
 * @param component_count Number of components the system requires
 * 
 * @note The rest of the parameters are the components the system requires
 * */
maybe_error_t maybe_world_register_system(
	maybe_world_t* world,
	maybe_system_function_t system_function,
	uint32_t component_count,
	...
);

/*
 * @brief Run one logic cycle of all systems in a world
 *
 * @param world A pointer to the world
 * */
maybe_error_t maybe_world_update(
	maybe_world_t* world
);

/*
 * @brief Free an ECS world's resources
 *
 * @param world A pointer to the world
 * */
maybe_error_t maybe_world_free(
	maybe_world_t* world
);

/* Macros for more comfortable use */
#define MAYBE_COMPONENT_ID(component) maybe_component_id__##component

#define MAYBE_DEFINE_COMPONENT_TYPE(component) uint32_t MAYBE_COMPONENT_ID(component) = 0;

#define MAYBE_REGISTER_COMPONENT_TYPE(world, component) \
	{\
		maybe_world_add_component_type((world), sizeof(component), &MAYBE_COMPONENT_ID(component)); \
	}
