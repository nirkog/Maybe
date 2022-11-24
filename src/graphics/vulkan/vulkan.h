#pragma once

#include <vulkan/vulkan.h>

#include "common/error.h"
#include "queue.h"

typedef VkBool32 (*maybe_vulkan_debug_messenger_callback_t)(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

/* A callback function that checks whether a physical device is suitable for use */
typedef bool (*device_picking_callback)(
	VkPhysicalDevice,
	VkSurfaceKHR,
	VkPhysicalDeviceProperties*,
	VkPhysicalDeviceFeatures*,
	maybe_vulkan_queue_device_supported_queue_indices_t*
);

/*
 * @brief Create a vulkan instane
 *
 * @param instance A pointer to the instance that will be initialized
 * @param extensions A list of vulkan extensions to be used by the instance
 * @param extension_count Number of extensions
 * @param validation_layers A list of vulkan validation layers to be used by the instance
 * @param validation_layer_count Number of validation layers
 * */
maybe_error_t maybe_vulkan_create_instance(
	VkInstance* instance,
	const char** extensions,
	uint32_t extension_count,
	const char** validation_layers,
	uint32_t validation_layer_count
); 

/*
 * @brief Setup a Vulkan debug messenger with a user callback
 *
 * @param instance A Vulkan instance
 * @param messenger The messenger to be initialized
 * @param callback A user defined callback function
 * */
maybe_error_t maybe_vulkan_setup_debug_messagner(
	VkInstance* instance,
	VkDebugUtilsMessengerEXT* messenger,
	maybe_vulkan_debug_messenger_callback_t callback
);

/*
 * @brief Destory a Vulkan debug messenger
 *
 * @param instance A Vulkan instance
 * @param messenger The messenger to be destroyed
 * */
void maybe_vulkan_destory_debug_messenger(
	VkInstance* instance,
	VkDebugUtilsMessengerEXT* messenger
);

/*
 * @brief Find the first suitable physical device for the engine use, using a user defined
 * 		  check
 *
 * @param instance A Vulkan instance
 * @param surface A Vulkan surface
 * @param callback A function that checks whether a certain device is suitable
 * 		  for use. If this argument is NULL, a default callback that only checks
 * 		  if the device is discrete GPU will be used
 * @param device The chosen device
 * @param proerties Chosen device's properties, can be NULL
 * @param features Chosen device's features, can be NULL
 * @param supported_queue_family_indices Indices of the supported queue families of the chosen device,
 * 		  can be NULL
 * */
maybe_error_t maybe_vulkan_find_suitable_physical_device(
	VkInstance* instance,
	VkSurfaceKHR surface,
	device_picking_callback callback,
	VkPhysicalDevice* device,
	VkPhysicalDeviceProperties* properties,
	VkPhysicalDeviceFeatures* features,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_family_indices
);

/*
 * @brief Enumerate the available extensions for a Vulkan physical device
 *
 * @param device The physical device
 * @param extensions The avialble extensions
 * @param count Number of available extensions
 *
 * @note extensions must be freed after this functions is called
 * */
maybe_error_t maybe_vulkan_get_physical_device_available_extensions(
	VkPhysicalDevice device,
	VkExtensionProperties** extensions,
	uint32_t* count
);

/*
 * @brief Create a Vulkan logical device that is attached to a physical device
 *
 * @param physical_device The physical device that will be attached to the logical device
 * @param device The new logical device
 * @param supported_queue_family_indices Indices of supported queue families in the physical
 * 	      device's queue list
 * @param extensions Extensions for the device
 * @param extension_count Number of extensions
 * */
maybe_error_t maybe_vulkan_create_logical_device(
	VkPhysicalDevice physical_device,
	VkDevice* device,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_family_indices,
	const char** extensions,
	uint32_t extension_count
);

/*
 * @brief Create an image view for an image
 *
 * @param device A Vulkan device
 * @param iamge The image
 * @param image_fromat The image format
 * @param image_view The resulting image view
 *
 * @note This function creates a very certain image view, and it
 * 		 should be expanded to be more generic in the future
 * */
maybe_error_t maybe_vulkan_create_image_view(
	VkDevice device,
	VkImage image,
	VkFormat image_format,
	VkImageView* image_view
);
