#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "common/error.h"

#include "list.h"

maybe_error_t maybe_list_init(
	maybe_list_t* list
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	if (NULL == list) {
		result = MAYBE_ERROR_LIST_NULL_PARAM;
		goto l_cleanup;
	}

	list->head = NULL;
	list->tail = NULL;
	list->count = 0;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_list_add(
	maybe_list_t* list,
	uint32_t node_size,
	void** node
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_list_node_t* new_node = NULL;
	
	if (NULL == list) {
		result = MAYBE_ERROR_LIST_NULL_PARAM;
		goto l_cleanup;
	}

	/* Allocate memory for the new node and its data */
	new_node = (maybe_list_node_t*)malloc(sizeof(maybe_list_node_t) - sizeof(list->head->data) + node_size);
	if (NULL == new_node) {
		result = MAYBE_ERROR_LIST_ALLOCATION_FAILED;
		goto l_cleanup;
	}

	new_node->next = NULL;
	new_node->prev = list->tail;

	/* Add node to the list */
	if (NULL == list->head) {
		list->head = new_node;
		list->tail = new_node;
	} else {
		list->tail->next = new_node;
		list->tail = new_node;
	}

	*node = (void*)&new_node->data;

	list->count++;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_list_remove(
	maybe_list_t* list,
	maybe_list_node_t* node
) {
	
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	
	if ((NULL == list) || (NULL == node)) {
		result = MAYBE_ERROR_LIST_NULL_PARAM;
		goto l_cleanup;
	}

	/* Remove node */
	if (list->head == node) {
		list->head = node->next;
	} else if (list->tail == node) {
		list->tail = node->prev;
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	free(node);
	list->count--;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_list_free(
	maybe_list_t* list
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_list_node_t* current_node = NULL;
	maybe_list_node_t* prev_node = NULL;
	
	if (NULL == list) {
		result = MAYBE_ERROR_LIST_NULL_PARAM;
		goto l_cleanup;
	}

	/* Iterate over the list and free each node */
	for (current_node = list->head; current_node != NULL;) {
		prev_node = current_node;
		current_node = current_node->next;

		free(prev_node);
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
