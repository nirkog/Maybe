#pragma once

static bool check_validation_layer_support(
	const char** user_layers,
	uint32_t user_layer_count
);

static bool default_physical_device_check_callback(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	VkPhysicalDeviceProperties* properties,
	VkPhysicalDeviceFeatures* features,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_family_indices
);

