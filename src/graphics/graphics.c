#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "utils/utils.h"
#include "logger/logger.h"
#include "vulkan/pipeline.h"
#include "vulkan/vulkan.h"
#include "vulkan/queue.h"
#include "vulkan/swap_chain.h"
#include "vulkan/pipeline.h"
#include "window.h"

#include "graphics.h"
#include "graphics_internal.h"

static maybe_window_t g_window;

static maybe_vulkan_queue_device_supported_queue_indices_t g_device_supported_queue_family_indices;

static VkInstance g_instance;
static VkSurfaceKHR g_surface;
static VkDebugUtilsMessengerEXT g_debug_messenger;
static VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;
static VkDevice g_device = VK_NULL_HANDLE;
static VkSwapchainKHR g_swap_chain;
static VkFormat g_format;
static VkExtent2D g_extent;
static MAYBE_VECTOR(VkImage) g_swap_chain_images;
static MAYBE_VECTOR(VkImageView) g_swap_chain_image_views;
static VkPipelineLayout g_pipeline_layout;
static VkRenderPass g_render_pass;
static VkPipeline g_graphics_pipeline;
static MAYBE_VECTOR(VkFramebuffer) g_swap_chain_framebuffers;
static VkCommandPool g_command_pool;
static VkCommandBuffer g_command_buffer;
static VkSemaphore g_image_available_semaphore;
static VkSemaphore g_render_finished_semaphore;
static VkFence g_in_flight_fence;

static VkQueue g_graphics_queue;
static VkQueue g_present_queue;


maybe_error_t maybe_graphics_init(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_vector_t extensions;
	VkPhysicalDeviceProperties device_properties;
	const char* validation_layers[] = {
#ifdef DEBUG
		"VK_LAYER_KHRONOS_validation"
#endif
	};
	const char* device_extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	uint32_t i;

	maybe_vector_init(&extensions, sizeof(const char*), 0);

	/* Initialize glfw */
	if (!glfwInit()) {
		result = MAYBE_ERROR_GRAPHICS_GLFW_INIT_FAILED;
		goto l_cleanup;
	}

	/* Check if vulkan is supported */
	if (!glfwVulkanSupported()) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_NOT_SUPPORTED;
		goto l_cleanup;
	}

	/* Create a window */
	result = maybe_window_init(&g_window, "Hello world!", 640, 480);
	if (IS_FAILURE(result)) {
		MAYBE_ERROR_LOG("Failed to create a window");
		goto l_cleanup;
	}

	get_required_vulkan_extensions(&extensions);

	/* Create a vulkan instance */
	result = maybe_vulkan_create_instance(
		&g_instance,
		MAYBE_VECTOR_DATA(extensions, const char*),
		extensions.length,
		validation_layers,
		LENGTH(validation_layers)
	);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

#ifdef DEBUG
	/* Setup the validation layers debug messenger */
	result = maybe_vulkan_setup_debug_messagner(&g_instance, &g_debug_messenger, debug_callback);
	if (IS_FAILURE(result)) {
		goto l_cleanup;		
	}
#endif

	/* Create a Vulkan surface */
	if (glfwCreateWindowSurface(g_instance, g_window.window, NULL, &g_surface) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_SURFACE_CREATION_FAILED;
		goto l_cleanup;
	}

	/* Set up a physical device */
	result = maybe_vulkan_find_suitable_physical_device(
		&g_instance,
		g_surface,
		NULL,
		&g_physical_device,
		&device_properties,
		NULL,
		&g_device_supported_queue_family_indices
	);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	/* @TODO Change this to log info */
	MAYBE_INFO_LOG("Using GPU {0s}", device_properties.deviceName);

	/* Set up a logical device */
	result = maybe_vulkan_create_logical_device(
		g_physical_device,
		&g_device,
		&g_device_supported_queue_family_indices,
		device_extensions,
		LENGTH(device_extensions)
	);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	/* Get handles to the device queues */
	vkGetDeviceQueue(g_device, g_device_supported_queue_family_indices.graphics_queue, 0, &g_graphics_queue);
	vkGetDeviceQueue(g_device, g_device_supported_queue_family_indices.present_queue, 0, &g_present_queue);

	/* Create a swap chain */
	CLEANUP_ON_FAILURE(create_swap_chain());

	/* Create an image view for each swap chain image */
	CLEANUP_ON_FAILURE(MAYBE_VECTOR_INIT(g_swap_chain_image_views, VkImageView));
	CLEANUP_ON_FAILURE(maybe_vector_resize(&g_swap_chain_image_views, g_swap_chain_images.length));
	for (i = 0; i < g_swap_chain_images.length; i++) {
		CLEANUP_ON_FAILURE(maybe_vulkan_create_image_view(
			g_device,
		   	MAYBE_VECTOR_ELEMENT(g_swap_chain_images, VkImage, i),
			g_format,
			&MAYBE_VECTOR_ELEMENT(g_swap_chain_image_views, VkImageView, i)
		));
	}

	CLEANUP_ON_FAILURE(create_render_pass());

	CLEANUP_ON_FAILURE(create_pipeline());

	CLEANUP_ON_FAILURE(create_framebuffers());

	CLEANUP_ON_FAILURE(create_command_pool());

	CLEANUP_ON_FAILURE(create_command_buffer());

	CLEANUP_ON_FAILURE(create_sync_objects());

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	maybe_vector_free(&extensions);

	return result;
}

void maybe_graphics_poll_events(void) {
	glfwPollEvents();
}

void maybe_graphics_terminate(void) {
	uint32_t i;
    
    vkDeviceWaitIdle(g_device);

    vkDestroySemaphore(g_device, g_image_available_semaphore, NULL);
    vkDestroySemaphore(g_device, g_render_finished_semaphore, NULL);
    vkDestroyFence(g_device, g_in_flight_fence, NULL);

	vkDestroyCommandPool(g_device, g_command_pool, NULL);

	/* Destroy all frame buffers */
	for (i = 0; i < g_swap_chain_framebuffers.length; i++) {
		vkDestroyFramebuffer(g_device, MAYBE_VECTOR_ELEMENT(g_swap_chain_framebuffers, VkFramebuffer, i), NULL);
	}

	/* Destroy all Vulkan objects */
	for (i = 0; i < g_swap_chain_image_views.length; i++) {
		 vkDestroyImageView(g_device, MAYBE_VECTOR_ELEMENT(g_swap_chain_image_views, VkImageView, i), NULL);
	}

#ifdef DEBUG
	maybe_vulkan_destory_debug_messenger(&g_instance, &g_debug_messenger);
#endif
	vkDestroyPipeline(g_device, g_graphics_pipeline, NULL);
	vkDestroyPipelineLayout(g_device, g_pipeline_layout, NULL);
	vkDestroyRenderPass(g_device, g_render_pass, NULL);
	vkDestroySwapchainKHR(g_device, g_swap_chain, NULL);
	vkDestroyDevice(g_device, NULL);
	vkDestroySurfaceKHR(g_instance, g_surface, NULL);
	vkDestroyInstance(g_instance, NULL);

	maybe_window_free(&g_window);

	glfwTerminate();

	/* Free all vectors */
	maybe_vector_free(&g_swap_chain_images);
	maybe_vector_free(&g_swap_chain_image_views);
	maybe_vector_free(&g_swap_chain_framebuffers);
}

bool maybe_graphics_should_shutdown(void) {
	return maybe_window_should_be_closed(&g_window);
}

void maybe_graphics_draw_frame(void) {
    VkSubmitInfo submit_info = { 0 };
    VkSemaphore wait_semaphores[] = { g_image_available_semaphore };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[] = { g_render_finished_semaphore };
    VkPresentInfoKHR present_info = { 0 };
    VkSwapchainKHR swap_chains[] = { g_swap_chain };
    uint32_t image_index;

    vkWaitForFences(g_device, 1, &g_in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(g_device, 1, &g_in_flight_fence); 

    vkAcquireNextImageKHR(g_device, g_swap_chain, UINT64_MAX, g_image_available_semaphore, VK_NULL_HANDLE, &image_index);

    vkResetCommandBuffer(g_command_buffer, 0);
    record_command_buffer(g_command_buffer, image_index);

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &g_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(g_graphics_queue, 1, &submit_info, g_in_flight_fence) != VK_SUCCESS) {
        MAYBE_ERROR_LOG("Queue submit error");
    }

    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    vkQueuePresentKHR(g_present_queue, &present_info);
}

void get_required_vulkan_extensions(
	maybe_vector_t* extensions
){
	const char** glfw_extensions = NULL;
	uint32_t glfw_extension_count = 0;
	uint32_t i;
	const char* custom_extensions[] = {
#ifdef DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	for (i = 0; i < glfw_extension_count; i++) {
		maybe_vector_push(extensions, &(glfw_extensions[i]));
	}

	for (i = glfw_extension_count; i < (glfw_extension_count + LENGTH(custom_extensions)); i++) {
		maybe_vector_push(extensions, &(custom_extensions[i - glfw_extension_count]));
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
		printf("validation layer: %s\n", pCallbackData->pMessage);
	}

    return VK_FALSE;
}

maybe_error_t create_swap_chain(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	maybe_swap_chain_support_details_t* swap_chain_support_details = NULL;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR present_mode;
	VkExtent2D extent;
	uint32_t image_count;

	CLEANUP_ON_FAILURE(maybe_vulkan_swap_chain_get_support_details(g_physical_device, g_surface, &swap_chain_support_details));

	format = choose_swap_surface_format(&swap_chain_support_details->formats);		
	present_mode = choose_swap_present_mode(&swap_chain_support_details->present_modes);
	extent = choose_swap_extent(&swap_chain_support_details->capabilities);

	/* Calculate swap chain image count */
	image_count = swap_chain_support_details->capabilities.minImageCount + 1;
	if ((swap_chain_support_details->capabilities.maxImageCount > 0) && (image_count > swap_chain_support_details->capabilities.maxImageCount)) {
		image_count = swap_chain_support_details->capabilities.maxImageCount;
	}

	/* Create the swap chain */
	CLEANUP_ON_FAILURE(maybe_vulkan_swap_chain_create(
		g_device,
		g_surface,
		swap_chain_support_details,
		&format,
		&present_mode,
		&extent,
		image_count,
		&g_device_supported_queue_family_indices,
		&g_swap_chain
	));

	/* Store swap chain image handles */
	CLEANUP_ON_FAILURE(maybe_vector_init(&g_swap_chain_images, sizeof(VkImage), 0));
	vkGetSwapchainImagesKHR(g_device, g_swap_chain, &image_count, NULL);
	CLEANUP_ON_FAILURE(maybe_vector_resize(&g_swap_chain_images, image_count));
	vkGetSwapchainImagesKHR(g_device, g_swap_chain, &image_count, MAYBE_VECTOR_DATA(g_swap_chain_images, VkImage));

	g_format = format.format;
	g_extent = extent;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	if (NULL != swap_chain_support_details) {
		maybe_vulkan_swap_chain_free_support_details(swap_chain_support_details);
	}

	return result;
}

VkSurfaceFormatKHR choose_swap_surface_format(
	MAYBE_VECTOR_PTR(VkSurfaceFormatKHR) available_formats
) {
	uint32_t i;
	VkSurfaceFormatKHR* current_format;

	for (i = 0; i < available_formats->length; i++) {
		current_format = &MAYBE_VECTOR_PTR_ELEMENT(available_formats, VkSurfaceFormatKHR, i); 
		if ((current_format->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) &&
		 	(current_format->format == VK_FORMAT_B8G8R8A8_SRGB)) {
			return *current_format;
		}
	}

	return MAYBE_VECTOR_PTR_ELEMENT(available_formats, VkSurfaceFormatKHR, 0); 
}

VkPresentModeKHR choose_swap_present_mode(
	MAYBE_VECTOR_PTR(VkPresentModeKHR) available_modes
) {
	uint32_t i;
	VkPresentModeKHR current_mode;

	for (i = 0; i < available_modes->length; i++) {
		current_mode = MAYBE_VECTOR_PTR_ELEMENT(available_modes, VkPresentModeKHR, i); 
		if (current_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return current_mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; 
}

VkExtent2D choose_swap_extent(
	const VkSurfaceCapabilitiesKHR* capabilities
) {
	int width, height;
	VkExtent2D actualExtent;

	if (capabilities->currentExtent.width != UINT_MAX) {
		return capabilities->currentExtent;
	}

	glfwGetFramebufferSize(g_window.window, &width, &height);

	actualExtent.width = maybe_utils_clamp_u32((uint32_t)width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
	actualExtent.height = maybe_utils_clamp_u32((uint32_t)height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

	return actualExtent;
}

maybe_error_t create_render_pass(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkAttachmentDescription color_attachment = { 0 };
	VkAttachmentReference color_attachment_ref = { 0 };
	VkSubpassDescription subpass = { 0 };
	VkRenderPassCreateInfo render_pass_info = { 0 };
    VkSubpassDependency dependency = { 0 };

    color_attachment.format = g_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;	
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(g_device, &render_pass_info, NULL, &g_render_pass) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_RENDER_PASS_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t create_pipeline(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkPipelineVertexInputStateCreateInfo vertex_input_info = { 0 };
	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	VkViewport viewport = { 0 };
	VkRect2D scissor = { 0 };
	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
	VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	VkPipelineColorBlendStateCreateInfo color_blending = { 0 };
	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	VkShaderModule vert_shader_module = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo vert_shader_create_info = { 0 };
	VkShaderModule frag_shader_module = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo frag_shader_create_info = { 0 };
	VkPipelineShaderStageCreateInfo shader_stages[2] = { 0 };
	VkGraphicsPipelineCreateInfo pipeline_info = { 0 };
	VkDynamicState dynamic_states[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state = { 0 };

	/* Setup vertex shader */
	CLEANUP_ON_FAILURE(maybe_vulkan_pipeline_create_shader_stage(
		g_device,
		"shaders/vert.spv",
		MAYBE_VULKAN_SHADER_TYPE_VERTEX,
		NULL,
		&vert_shader_create_info,
		&vert_shader_module
	));

	/* Setup fragment shader */
	CLEANUP_ON_FAILURE(maybe_vulkan_pipeline_create_shader_stage(
		g_device,
		"shaders/frag.spv",
		MAYBE_VULKAN_SHADER_TYPE_FRAGMENT,
		NULL,
		&frag_shader_create_info,
		&frag_shader_module
	));

	memcpy(&shader_stages[0], &vert_shader_create_info, sizeof(shader_stages[0]));
	memcpy(&shader_stages[1], &frag_shader_create_info, sizeof(shader_stages[1]));

	/* Create input assembler stage info */
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = NULL;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = NULL;

	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	/* Setup viewport */
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)g_extent.width;
	viewport.height = (float)g_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	/* Setup scissor */
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = g_extent;  

	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = LENGTH(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;

	/* Setup dynamic viewport state */
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	/* Setup rasterization stage */
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	/* Setup multisampling */
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	/* Set up color blending */
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	/* Set up pipleine layout for uniforms */
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = NULL;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = NULL;
	if (vkCreatePipelineLayout(g_device, &pipeline_layout_info, NULL, &g_pipeline_layout) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_PIPELINE_LAYOUT_CREATION_FAILED;
		goto l_cleanup;
	}

	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = NULL;
   	pipeline_info.pColorBlendState = &color_blending;
   	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = g_pipeline_layout;
	pipeline_info.renderPass = g_render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  	pipeline_info.basePipelineIndex = -1;
	if (vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &g_graphics_pipeline) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHCIS_VULKAN_PIPELINE_CREATION_FAILED;
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	vkDestroyShaderModule(g_device, frag_shader_module, NULL);
	vkDestroyShaderModule(g_device, vert_shader_module, NULL);

	return result;
}

maybe_error_t create_framebuffers(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkFramebufferCreateInfo framebuffer_info = { 0 };
	uint32_t i;

	/* Initialize frame buffer vector */
	CLEANUP_ON_FAILURE(MAYBE_VECTOR_INIT(g_swap_chain_framebuffers, VkFramebuffer));
	CLEANUP_ON_FAILURE(maybe_vector_resize(&g_swap_chain_framebuffers, g_swap_chain_image_views.length));

	/* Create each frame buffer and attach it to a swap chain image */
	for (i = 0; i < g_swap_chain_image_views.length; i++) {
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = g_render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = &MAYBE_VECTOR_ELEMENT(g_swap_chain_image_views, VkImageView, i);
		framebuffer_info.width = g_extent.width;
		framebuffer_info.height = g_extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(g_device, &framebuffer_info, NULL, &MAYBE_VECTOR_ELEMENT(g_swap_chain_framebuffers, VkFramebuffer, i)) != VK_SUCCESS) {
			result = MAYBE_ERROR_GRAPHICS_VULKAN_FRAME_BUFFER_CREATION_FAILED;
			goto l_cleanup;
		}
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t create_command_pool(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;

	VkCommandPoolCreateInfo pool_info = { 0 };

	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = g_device_supported_queue_family_indices.graphics_queue;	

	if (vkCreateCommandPool(g_device, &pool_info, NULL, &g_command_pool) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_COMMAND_POOL_CREATION_FAILED;	
		goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

maybe_error_t create_command_buffer(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkCommandBufferAllocateInfo alloc_info = { 0 };

	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = g_command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(g_device, &alloc_info, &g_command_buffer) != VK_SUCCESS) {
        result = MAYBE_ERROR_GRAPHICS_VULKAN_COMMAND_BUFFER_CREATION_FAILED;
        goto l_cleanup;
	}

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

static maybe_error_t record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkCommandBufferBeginInfo begin_info = { 0 };
    VkRenderPassBeginInfo render_pass_info = { 0 };
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkViewport viewport = { 0 };
    VkRect2D scissor = { 0 };
    
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0; // Optional
	begin_info.pInheritanceInfo = NULL; // Optional

	if (vkBeginCommandBuffer(g_command_buffer, &begin_info) != VK_SUCCESS) {
        result = MAYBE_ERROR_GRAPHICS_VULKAN_COMMAND_BUFFER_RECORDING_FAILED;
        goto l_cleanup;    
	}

    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = g_render_pass;
    render_pass_info.framebuffer = MAYBE_VECTOR_ELEMENT(g_swap_chain_framebuffers, VkFramebuffer, image_index);
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent = g_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphics_pipeline);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)g_extent.width;
    viewport.height = (float)g_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = g_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        result = MAYBE_ERROR_GRAPHICS_VULKAN_COMMAND_BUFFER_RECORDING_FAILED;
        goto l_cleanup;    
    }

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}

static maybe_error_t create_sync_objects(void) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
    VkSemaphoreCreateInfo semaphore_info = { 0 };
    VkFenceCreateInfo fence_info = { 0 };

    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(g_device, &semaphore_info, NULL, &g_image_available_semaphore) != VK_SUCCESS ||
        vkCreateSemaphore(g_device, &semaphore_info, NULL, &g_render_finished_semaphore) != VK_SUCCESS ||
        vkCreateFence(g_device, &fence_info, NULL, &g_in_flight_fence) != VK_SUCCESS) {
        result = MAYBE_ERROR_GRAPHICS_VULKAN_SYNC_OBJECT_CREATION_FAILED;
        goto l_cleanup;
    }

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	return result;
}
