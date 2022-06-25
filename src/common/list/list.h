#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/error.h"

typedef struct maybe_list_node_s {
	struct maybe_list_node_s* next;
	struct maybe_list_node_s* prev;
	void* data; /* @note This is a placeholder for the real data to follow */
} maybe_list_node_t;

typedef struct {
	maybe_list_node_t* head;	
	maybe_list_node_t* tail;	
	uint32_t count;
} maybe_list_t;

/*
 * @brief Initialize a list
 *
 * @param list A pointer to the new list
 * */
maybe_error_t maybe_list_init(
	maybe_list_t* list
);

/*
 * @brief Add a node to a list
 *
 * @param list A pointer to the list
 * @param node_size Size of the node
 * @param node A pointer to the newly allocated node
 * */
maybe_error_t maybe_list_add(
	maybe_list_t* list,
	uint32_t node_size,
	void** node
);

/*
 * @brief Remove a node from a list
 *
 * @param list A pointer to the list
 * @param node A pointer to the node to be removed
 * */
maybe_error_t maybe_list_remove(
	maybe_list_t* list,
	maybe_list_node_t* node
);

/*
 * @brief Free a list's resources
 *
 * @param list A pointer to the list
 * */
maybe_error_t maybe_list_free(
	maybe_list_t* list
);
