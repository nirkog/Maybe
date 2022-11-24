#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

#include "common/error.h"

#define MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX (0xffffffff)

/* Indicate which queue families are supported by a physical device */
typedef struct {
	uint32_t graphics_queue;
	uint32_t present_queue;
} maybe_vulkan_queue_device_supported_queue_indices_t;

/*
 * @brief Find which queue families are supported by a physical device, and their indices in
 * 		  the device's queue list
 *
 * @param device A physical device
 * @param surface A Vulkan surface
 * @param supported_queues The supported queue family indices in the device's queue list
 * */
maybe_error_t maybe_vulkan_queue_get_device_queue_family_indices(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_indices
);
