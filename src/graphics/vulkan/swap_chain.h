#pragma once

#include <vulkan/vulkan.h>

#include "common/vector/vector.h"
#include "common/common.h"
#include "queue.h"

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	MAYBE_VECTOR(VkSurfaceFormatKHR) formats;
	MAYBE_VECTOR(VkPresentModeKHR) present_modes;
} maybe_swap_chain_support_details_t;

/*
 * @brief Get support information of a swap chain
 *
 * @param device A physical device
 * @param surface A Vulkan surface
 * @param support_details The support details
 *
 * @note support_details must be freed after use using vulkan_swap_chain_free_support_details
 * */
maybe_error_t maybe_vulkan_swap_chain_get_support_details(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	maybe_swap_chain_support_details_t** support_details
);

/*
 * @brief Free all resources used by a support details instance
 *
 * @param support_details A support details instance
 * */
void maybe_vulkan_swap_chain_free_support_details(
	maybe_swap_chain_support_details_t* support_details
);

/*
 * @brief Create a Vulkan swap chain
 *
 * @param device The logical device the swap chain will be attached to
 * @param surface The Vulkan surface the swap chain will be attached to
 * @param support_details Current swap chain support details
 * @param format Swap chain surface format
 * @param present_mode Swap chain present mode
 * @param extent Swap chain extent
 * @param supported_queue_indices Device's supported queue families indices
 * @param image_count Swap chain image count
 * */
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
);
