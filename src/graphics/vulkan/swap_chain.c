#include "swap_chain.h"

maybe_error_t maybe_vulkan_swap_chain_get_support_details(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	maybe_swap_chain_support_details_t** support_details
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_swap_chain_support_details_t* inner_support_details = NULL;
	uint32_t format_count, present_mode_count;

	if (NULL == support_details) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_SWAP_CHAIN_NULL_PARAM;
		goto l_cleanup;
	}

	/* Initialize support details */
	inner_support_details = MALLOC_T(maybe_swap_chain_support_details_t, 1);
	if (NULL == inner_support_details) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_SWAP_CHAIN_MALLOC_FAILED;
		goto l_cleanup;
	}

	CLEANUP_ON_FAILURE(maybe_vector_init(&inner_support_details->formats, sizeof(VkSurfaceFormatKHR), 0));
	CLEANUP_ON_FAILURE(maybe_vector_init(&inner_support_details->present_modes, sizeof(VkPresentModeKHR), 0));

	/* Query capabilities */
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &inner_support_details->capabilities);

	/* Query formats */
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);
	if (format_count != 0) {
		CLEANUP_ON_FAILURE(maybe_vector_resize(&inner_support_details->formats, format_count));
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, MAYBE_VECTOR_DATA(inner_support_details->formats, VkSurfaceFormatKHR));
	}

	/* Query present modes */
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);
	if (present_mode_count != 0) {
		CLEANUP_ON_FAILURE(maybe_vector_resize(&inner_support_details->present_modes, present_mode_count));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, MAYBE_VECTOR_DATA(inner_support_details->present_modes, VkPresentModeKHR));
	}

	*support_details = inner_support_details;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

void maybe_vulkan_swap_chain_free_support_details(
	maybe_swap_chain_support_details_t* support_details
) {
	if (NULL == support_details) {
		return;
	}

	maybe_vector_free(&support_details->formats);
	maybe_vector_free(&support_details->present_modes);
}

maybe_error_t maybe_vulkan_swap_chain_create(
	VkDevice device,
	VkSurfaceKHR surface,
	maybe_swap_chain_support_details_t* support_details,
	VkSurfaceFormatKHR* format,
	VkPresentModeKHR* present_mode,
	VkExtent2D* extent,
	uint32_t image_count,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_indices,
	VkSwapchainKHR* swap_chain
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkSwapchainCreateInfoKHR create_info = { 0 };
	uint32_t supported_queue_indices_array[] = { supported_queue_indices->graphics_queue, supported_queue_indices->present_queue };

	if ((NULL == format) || (NULL == present_mode) || (NULL == extent)) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_SWAP_CHAIN_NULL_PARAM;
		goto l_cleanup;
	}

	/* Set up swap chain user informatio */
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = format->format;
	create_info.imageColorSpace = format->colorSpace;
	create_info.imageExtent = *extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.preTransform = support_details->capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = *present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	/* If the present queue and the graphics queue are not the same use the concurrent sharing mode */
	if (supported_queue_indices->graphics_queue != supported_queue_indices->present_queue) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = supported_queue_indices_array;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = NULL;
	}

	/* Create swap chain */
	if (vkCreateSwapchainKHR(device, &create_info, NULL, swap_chain) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_SWAP_CHAIN_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
