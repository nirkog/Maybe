#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/error.h"
#include "common/list/list.h"

#define MAYBE_MAP_DEFAULT_CAPACITY (10)

/* @TODO move to internal */
typedef struct {
	bool is_empty; /* @TODO Think about optimizing this */
	bool contains_collisions;
	maybe_list_t list;
	bool list_initialized;
	void* key;
} maybe_map_entry_t;

typedef uint32_t (*maybe_hash_function_t)(uint8_t* buffer, uint32_t size);

typedef struct {
	maybe_hash_function_t hash_function;
	maybe_map_entry_t* entries;
	void* values; /* @TODO Change this to a vector? */
	uint32_t capacity;
	uint32_t element_size;
} maybe_map_t;

/*
 * @brief Initialize a map
 *
 * @param map A pointer to the new map
 * @param element_size The size of an element in the map
 * @param capacity The capacity of the map, if 0 the capacity will be set to a default value
 * @param hash_function The function used for hashing the keys
 * */
maybe_error_t maybe_map_init(
	maybe_map_t* map,
	uint32_t element_size,
	uint32_t capacity,
	maybe_hash_function_t hash_function
);

/*
 * @TODO Maybe return a pointer to the value
 * @brief Set a value for a key in a map
 *
 * @param map A pointer to the map
 * @param key The key data
 * @param key_size The size of the key's data
 * @param value The value to set
 * */
maybe_error_t maybe_map_set(
	maybe_map_t* map,
	void* key,
	uint32_t key_size,
	void* value
);

/*
 * @brief Get a value for a given key in a map
 *
 * @param map A pointer to the map
 * @param key The key data
 * @param key_size The size of the key's data
 * @param value A pointer to the value, NULL if key was not found
 * */
maybe_error_t maybe_map_get(
	maybe_map_t* map,
	void* key,
	uint32_t key_size,
	void** value
);

/*
 * @brief Remove a key-value pair from a map
 *
 * @param map A pointer to the map
 * @param key The key data
 * @param key_size The size of the key's data
 * */
maybe_error_t maybe_map_remove(
	maybe_map_t* map,
	void* key,
	uint32_t key_size
);

/* @TODO Add resize function */

/*
 * @brief Free a map's resources
 * */
maybe_error_t maybe_map_free(
	maybe_map_t* map
);

uint32_t maybe_map_default_hash_function(
	uint8_t* buffer,
	uint32_t size
);

#define MAYBE_MAP(key_type, value_type) maybe_map_t
