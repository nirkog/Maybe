#include <string.h>

#include "common/common.h"

#include "queue.h"

maybe_error_t maybe_vulkan_queue_get_device_queue_family_indices(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_indices
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t queue_family_count, i;
	VkQueueFamilyProperties* queue_families = NULL;
	VkBool32 present_support = VK_FALSE;

	if (NULL == supported_queue_indices) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_QUEUE_NULL_PARAM;
		goto l_cleanup;
	}

	/* Get all supported queue families */
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	queue_families = MALLOC_T(VkQueueFamilyProperties, queue_family_count);
	if (NULL == queue_families) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_QUEUE_MALLOC_FAILED;
	}
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	/* Initialize all indices to 0xffffffff which shouldn't be a valid index */
	memset(supported_queue_indices, (uint8_t)MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX, sizeof(maybe_vulkan_queue_device_supported_queue_indices_t));

	/* Check which queue families are supported */
	for (i = 0; i < queue_family_count; i++) {
		if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			(MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX == supported_queue_indices->graphics_queue)) {
			supported_queue_indices->graphics_queue = i;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
		if ((VK_TRUE == present_support) &&
			(MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX == supported_queue_indices->present_queue)) {
			supported_queue_indices->present_queue = i;
		}
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
