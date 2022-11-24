#include <stdbool.h>
#include <vulkan/vulkan.h>

#include "common/vector/vector.h"

static void get_required_vulkan_extensions(
	maybe_vector_t* extensions
);

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
);

static maybe_error_t create_swap_chain(void);

static VkSurfaceFormatKHR choose_swap_surface_format(
	MAYBE_VECTOR_PTR(VkSurfaceFormatKHR) available_formats
);

static VkPresentModeKHR choose_swap_present_mode(
	MAYBE_VECTOR_PTR(VkPresentModeKHR) available_formats
);
	
static VkExtent2D choose_swap_extent(
	const VkSurfaceCapabilitiesKHR* capabilities
);

static maybe_error_t create_render_pass(void);

static maybe_error_t create_pipeline(void);

static maybe_error_t create_framebuffers(void);

static maybe_error_t create_command_pool(void);

static maybe_error_t create_command_buffer(void);

static maybe_error_t record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);

static maybe_error_t create_sync_objects(void);
