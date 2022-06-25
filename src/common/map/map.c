#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "common/error.h"
#include "common/list/list.h"

#include "map.h"

maybe_error_t maybe_map_init(
	maybe_map_t* map,
	uint32_t element_size,
	uint32_t capacity,
	maybe_hash_function_t hash_function
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t i;

	if ((NULL == map) || (NULL == hash_function)) {
		result = MAYBE_ERROR_MAP_NULL_PARAM;
		goto l_cleanup;
	}

	map->hash_function = hash_function;
	map->capacity = capacity;
	map->element_size = element_size;

	map->values = malloc(element_size * capacity);
	if (NULL == map->values) {
		result = MAYBE_ERROR_MAP_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	map->entries = malloc(sizeof(maybe_map_entry_t) * capacity);
	if (NULL == map->entries) {
		result = MAYBE_ERROR_MAP_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	/* Initialize all entries */
	for (i = 0; i < map->capacity; i++) {
		map->entries[i].is_empty = true;
		map->entries[i].contains_collisions = false;
		map->entries[i].list_initialized = false;
		map->entries[i].key = NULL;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	if (IS_FAILURE(result)) {
		if (map->values) {
			free(map->values);
		}

		if (map->entries) {
			free(map->entries);
		}
	}

	return result;
}

maybe_error_t maybe_map_set(
	maybe_map_t* map,
	void* key,
	uint32_t key_size,
	void* value
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_map_entry_t* entry = NULL;
	void* node = NULL;
	uint32_t index = 0;

	if (NULL == map) {
		result = MAYBE_ERROR_MAP_NULL_PARAM;
		goto l_cleanup;
	}

	index = map->hash_function(key, key_size) % map->capacity;
	entry = &(map->entries[index]);

	if (entry->is_empty) {
		/* If the entry is empty, copy the key and and the value */
		entry->key = malloc(key_size);
		if (!entry->key) {
			result = MAYBE_ERROR_MAP_ALLOCATION_FAILED;
			goto l_cleanup;
		}

		memcpy(entry->key, key, key_size);

		entry->is_empty = false;

		memcpy((void*)((uint8_t*)map->values + (index * map->element_size)), value, map->element_size);
	} else {
		if (!entry->list_initialized) {
			/* If this is the first collision, initilize the collision list */
			result = maybe_list_init(&entry->list);
			if (IS_FAILURE(result)) {
				goto l_cleanup;
			}

			entry->contains_collisions = true;
			entry->list_initialized = true;
		}

		/* Allocate another node in the collision list */
		result = maybe_list_add(&entry->list, key_size + map->element_size, &node);
		if (IS_FAILURE(result)) {
			goto l_cleanup;
		}

		/*  Copy the key and value to the new node */
		memcpy(node, key, key_size);
		memcpy((void*)((uint8_t*)node + key_size), value, map->element_size);
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_map_get(
	maybe_map_t* map,
	void* key,
	uint32_t key_size,
	void** value
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_map_entry_t* entry = NULL;
	maybe_list_node_t* current_node = NULL;
	void* found_value = NULL;
	uint32_t index = 0;

	if ((NULL == map) || (NULL == value)) {
		result = MAYBE_ERROR_MAP_NULL_PARAM;
		goto l_cleanup;
	}

	index = map->hash_function(key, key_size) % map->capacity;
	entry = &(map->entries[index]);

	/* @TODO Export search to a function */
	if (!entry->is_empty) {
		if (entry->contains_collisions && entry->list_initialized) {
			for (current_node = entry->list.head; current_node != NULL;) {
				if (0 == memcmp(key, (void*)&current_node->data, key_size)) {
					found_value = (void*)((uint8_t*)&current_node->data + key_size);
					break;
				}
				
				current_node = current_node->next;
			}	
		} 

		if (entry->key && (NULL == found_value)) {
			if (0 == memcmp(key, entry->key, key_size)) {
				found_value = (void*)((uint8_t*)map->values + (index * map->element_size));
			}
		}
	}

	*value = found_value;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_map_remove(
	maybe_map_t* map,
	void* key,
	uint32_t key_size
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_map_entry_t* entry = NULL;
	maybe_list_node_t* current_node = NULL;
	void* found_value = NULL;
	uint32_t index = 0;

	if (NULL == map) {
		result = MAYBE_ERROR_MAP_NULL_PARAM;
		goto l_cleanup;
	}

	index = map->hash_function(key, key_size) % map->capacity;
	entry = &(map->entries[index]);

	if (!entry->is_empty) {
		if (entry->contains_collisions && entry->list_initialized) {
			for (current_node = entry->list.head; current_node != NULL;) {
				if (0 == memcmp(key, (void*)&current_node->data, key_size)) {
					(void)maybe_list_remove(&entry->list, current_node);

					if (entry->list.count == 0) {
						entry->contains_collisions = false;
					}

					break;
				}
				
				current_node = current_node->next;
			}	
		} 

		if (entry->key && (NULL == found_value)) {
			if (0 == memcmp(key, entry->key, key_size)) {
				free(entry->key);
				entry->key = NULL;

				if (!entry->list_initialized || (entry->list.count == 0)) {
					entry->is_empty = true;
				}
			}
		}
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_map_free(
	maybe_map_t* map
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t i;

	if (NULL == map) {
		result = MAYBE_ERROR_MAP_NULL_PARAM;
		goto l_cleanup;
	}

	if (map->values) {
		free(map->values);
	}

	/* Free all used collision lists and entry keys */
	if (map->entries) {
		for (i = 0; i < map->capacity; i++) {
			if (map->entries[i].list_initialized) {
				maybe_list_free(&map->entries[i].list);
			}

			if (map->entries[i].key) {
				free(map->entries[i].key);	
			}
		}
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

uint32_t maybe_map_default_hash_function(
	uint8_t* buffer,
	uint32_t size
) {
	uint32_t i = 0;
	uint32_t result = 0;
	uint32_t remainder = 0;

	for (i = 0; i < size; i += 4) {
		if ((i + sizeof(uint32_t)) > size) {
			break;
		}
		result += *(uint32_t*)(&buffer[i]);
	}

	/* If there is a remainder, add it */
	if (size % sizeof(uint32_t) != 0) {
		memcpy(&remainder, &buffer[i], size - i);
		result += remainder;
	}

	return result;
}
