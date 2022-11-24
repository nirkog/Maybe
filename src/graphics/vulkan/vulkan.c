#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/common.h"
#include "logger/logger.h"
#include "common/vector/vector.h"
#include "extension_functions.h"
#include "queue.h"
#include "swap_chain.h"

#include "vulkan.h"
#include "vulkan_internal.h"

maybe_error_t maybe_vulkan_create_instance(
	VkInstance* instance,
	const char** extensions,
	uint32_t extension_count,
	const char** validation_layers,
	uint32_t validation_layer_count
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkApplicationInfo app_info = { 0 };
	VkInstanceCreateInfo create_info = { 0 };

	if ((NULL == instance) || (NULL == extensions) || (NULL == validation_layers)) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NULL_PARAM;
		goto l_cleanup;
	}

	/* Make sure all selected validation layers are supported */
	if (!check_validation_layer_support(validation_layers, validation_layer_count)) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_UNSUPPORTED_VALIDATION_LAYERS;
		goto l_cleanup;
	}

	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Maybe Engine Application";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Maybe Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	create_info.enabledExtensionCount = extension_count;
	create_info.ppEnabledExtensionNames = extensions;

	create_info.enabledLayerCount = validation_layer_count;
	create_info.ppEnabledLayerNames = validation_layers;

	/* Create instance */
	if (vkCreateInstance(&create_info, NULL, instance) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_INSTANCE_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vulkan_setup_debug_messagner(
	VkInstance* instance,
	VkDebugUtilsMessengerEXT* messenger,
	maybe_vulkan_debug_messenger_callback_t callback
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;

	if ((NULL == instance) || (NULL == messenger) || (NULL == callback)) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NULL_PARAM;
		goto l_cleanup;
	}

	/* Initialize messenger settings */
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = callback;
	createInfo.pUserData = NULL;
	createInfo.pNext = NULL;
	createInfo.flags = 0;

	if (CreateDebugUtilsMessengerEXT(*instance, &createInfo, NULL, messenger) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_DEBUG_MESSANGER_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

void maybe_vulkan_destory_debug_messenger(
	VkInstance* instance,
	VkDebugUtilsMessengerEXT* messenger
) {
	DestroyDebugUtilsMessengerEXT(*instance, *messenger, NULL);
}

maybe_error_t maybe_vulkan_find_suitable_physical_device(
	VkInstance* instance,
	VkSurfaceKHR surface,
	device_picking_callback callback,
	VkPhysicalDevice* device,
	VkPhysicalDeviceProperties* properties,
	VkPhysicalDeviceFeatures* features,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_family_indices
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	uint32_t i;
	uint32_t device_count = 0;
	VkPhysicalDevice* devices = NULL;
	VkPhysicalDevice chosen_device = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceFeatures device_features;
	maybe_vulkan_queue_device_supported_queue_indices_t device_supported_queue_family_indices;

	if ((NULL == instance) || (NULL == device)) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NULL_PARAM;
		goto l_cleanup;
	}

	/* If no callback was specified, use a default one */
	if (NULL == callback) {
		callback = default_physical_device_check_callback;
	}

	/* Get all available physical devices */
	vkEnumeratePhysicalDevices(*instance, &device_count, NULL);
	devices = MALLOC_T(VkPhysicalDevice, device_count);
	if (NULL == devices) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_MALLOC_FAILED;
		goto l_cleanup;
	}
	vkEnumeratePhysicalDevices(*instance, &device_count, devices);

	/* Find a suitable device */
	for (i = 0; i < device_count; i++) {
		/* Get device information */
		vkGetPhysicalDeviceProperties(devices[i], &device_properties);
		vkGetPhysicalDeviceFeatures(devices[i], &device_features);

		/* Get the supported family queue indices for each device so we can save them for the chosen one */
		result = maybe_vulkan_queue_get_device_queue_family_indices(devices[i], surface, &device_supported_queue_family_indices);
		if (IS_FAILURE(result)) {
			goto l_cleanup;
		}

		/* Check if device is suitable */
		if (callback(devices[i], surface, &device_properties, &device_features, &device_supported_queue_family_indices)) {
			chosen_device = devices[i];
			break;	
		}
	}

	/* Check if a device was found */
	if (VK_NULL_HANDLE == chosen_device) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NO_SUITABLE_DEVICE;
		goto l_cleanup;
	}

	*device = chosen_device;
	if (NULL != properties) {
		memcpy((void*)properties, (const void*)&device_properties, sizeof(device_properties));
	}
	if (NULL != features) {
		memcpy((void*)features, (const void*)&device_features, sizeof(device_features));
	}
	if (NULL != supported_queue_family_indices) {
		memcpy((void*)supported_queue_family_indices, (const void*)&device_supported_queue_family_indices, sizeof(device_supported_queue_family_indices));
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vulkan_get_physical_device_available_extensions(
	VkPhysicalDevice device,
	VkExtensionProperties** extensions,
	uint32_t* count
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkExtensionProperties* inner_extensions = NULL;
	uint32_t inner_count = 0; 

	if ((NULL == extensions) || (NULL == count)) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NULL_PARAM;
		goto l_cleanup;
	}

	/* Allocate memory for the extensions */
	vkEnumerateDeviceExtensionProperties(device, NULL, &inner_count, NULL);
	inner_extensions = MALLOC_T(VkExtensionProperties, inner_count);
	if (NULL == inner_extensions) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_MALLOC_FAILED;
		goto l_cleanup;
	}

	vkEnumerateDeviceExtensionProperties(device, NULL, &inner_count, inner_extensions);

	*extensions = inner_extensions;
	*count = inner_count;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vulkan_create_logical_device(
	VkPhysicalDevice physical_device,
	VkDevice* device,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_family_indices,
	const char** extensions,
	uint32_t extension_count
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	MAYBE_VECTOR(VkDeviceQueueCreateInfo) queues_create_info;
	float graphics_queue_priority = 1.0f;
	VkPhysicalDeviceFeatures device_features = { 0 };
	VkDeviceCreateInfo device_create_info = { 0 };
	uint32_t queue_family_indices[2] = { MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX };
	uint32_t used_queue_family_indices[2] = { MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX };
	uint32_t i, j;
	bool duplicate = false;

	if ((NULL == device) || ((NULL == extensions) && (extension_count != 0))) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NULL_PARAM;
		goto l_cleanup;
	}

	memset(used_queue_family_indices, MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX, sizeof(used_queue_family_indices));
	queue_family_indices[0] = supported_queue_family_indices->graphics_queue;
	queue_family_indices[1] = supported_queue_family_indices->present_queue;

	CLEANUP_ON_FAILURE(maybe_vector_init(&queues_create_info, sizeof(VkDeviceQueueCreateInfo), 0));

	/* Create the required queues (make sure there is only one of each queue)  */
	for (i = 0; i < LENGTH(queue_family_indices); i++) {
		duplicate = false;
		for (j = 0; j < LENGTH(used_queue_family_indices); j++) {
			if (used_queue_family_indices[j] == queue_family_indices[i]) {
				duplicate = true;
				break;
			}
		}	

		if (duplicate) {
			continue;
		}

		maybe_vector_push(&queues_create_info, NULL);
		memset(&MAYBE_VECTOR_ELEMENT(queues_create_info, VkDeviceQueueCreateInfo, 0), 0, sizeof(VkDeviceQueueCreateInfo));
		MAYBE_VECTOR_ELEMENT(queues_create_info, VkDeviceQueueCreateInfo, 0).sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		MAYBE_VECTOR_ELEMENT(queues_create_info, VkDeviceQueueCreateInfo, 0).queueFamilyIndex = queue_family_indices[i];
		MAYBE_VECTOR_ELEMENT(queues_create_info, VkDeviceQueueCreateInfo, 0).queueCount = 1;
		MAYBE_VECTOR_ELEMENT(queues_create_info, VkDeviceQueueCreateInfo, 0).pQueuePriorities = &graphics_queue_priority;
		used_queue_family_indices[i] = queue_family_indices[i];
	}

	/* Initialize device create information */
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = MAYBE_VECTOR_DATA(queues_create_info, VkDeviceQueueCreateInfo);
	device_create_info.queueCreateInfoCount = queues_create_info.length;
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.enabledExtensionCount = extension_count;
	device_create_info.ppEnabledExtensionNames = extensions;
	device_create_info.enabledLayerCount = 0;

	if (vkCreateDevice(physical_device, &device_create_info, NULL, device) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_DEVICE_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t maybe_vulkan_create_image_view(
	VkDevice device,
	VkImage image,
	VkFormat image_format,
	VkImageView* image_view
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkImageViewCreateInfo create_info = { 0 };

	/* Initialize create info */
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = image_format;
	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;
	
	if (vkCreateImageView(device, &create_info, NULL, image_view) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_IMAGE_VIEW_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

bool check_validation_layer_support(
	const char** user_layers,
	uint32_t user_layer_count
) {
	uint32_t i, j;
	bool found_layer = false;
	uint32_t layer_count;
	VkLayerProperties* available_layers = NULL;
	VkLayerProperties* current_layer = NULL;

	/* Allocate memory for the available validation layers properties */
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	available_layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layer_count);
	if (NULL == available_layers) {
		return false;
	}

	/* Get the available validation layers */
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

	/* Try to find each of the wanted validation layers in the supported ones */
	for (i = 0; i < user_layer_count; i++) {
		found_layer = false;

		for (j = 0; j < layer_count; j++) {
			current_layer = &available_layers[j];

			if (0 == strcmp(current_layer->layerName, user_layers[i])) {
				found_layer = true;
				break;
			}
		}

		if (!found_layer) {
			return false;
		}
	}

	return true;
}

static bool default_physical_device_check_callback(
	VkPhysicalDevice device,
	VkSurfaceKHR surface,
	VkPhysicalDeviceProperties* properties,
	VkPhysicalDeviceFeatures* features,
	maybe_vulkan_queue_device_supported_queue_indices_t* supported_queue_family_indices
) {
	bool result = false;
	VkExtensionProperties* device_extensions;
	uint32_t extension_count = 0, i, j, found_extensions = 0;
	const char* required_extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	maybe_swap_chain_support_details_t* swap_chain_support_details = NULL;

	if (IS_FAILURE(maybe_vulkan_get_physical_device_available_extensions(device, &device_extensions, &extension_count))) {
		goto l_cleanup;
	}

	/* Validate that all required extensions are available */
	for (i = 0; i < extension_count; i++) {
		for (j = 0; j < LENGTH(required_extensions); j++) {
			if (0 == strcmp(device_extensions[i].extensionName, required_extensions[j])) {
				found_extensions++;
				break;
			}
		}
	}

	if (found_extensions != LENGTH(required_extensions)) {
		goto l_cleanup;
	}

	/* Validate swap chain supports our use */
	if (IS_FAILURE(maybe_vulkan_swap_chain_get_support_details(device, surface, &swap_chain_support_details))) {
		goto l_cleanup;
	}

	if (MAYBE_VECTOR_EMPTY(swap_chain_support_details->formats) || MAYBE_VECTOR_EMPTY(swap_chain_support_details->present_modes)) {
		goto l_cleanup;
	}
	
	/* Validate that the device is a discrete graphics card */
	if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		goto l_cleanup;
	}

	/* Validate that graphics queues are supported */
	if (MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX == supported_queue_family_indices->graphics_queue) {
		goto l_cleanup;
	}

	/* Validate that present queues are supported */
	if (MAYBE_VULKAN_QUEUE_INVALID_QUEUE_INDEX == supported_queue_family_indices->present_queue) {
		goto l_cleanup;
	}

	result = true;
l_cleanup:
	if (NULL != device_extensions) {
		free(device_extensions);
	}

	if (NULL != swap_chain_support_details) {
		maybe_vulkan_swap_chain_free_support_details(swap_chain_support_details);
	}

	return result;
}
