#include "pipeline_internal.h"
#include "utils/io.h"

#include "pipeline.h"
#include <vulkan/vulkan_core.h>

VkShaderStageFlagBits g_shader_type_to_stage[] = {
	[MAYBE_VULKAN_SHADER_TYPE_VERTEX] = VK_SHADER_STAGE_VERTEX_BIT,
	[MAYBE_VULKAN_SHADER_TYPE_FRAGMENT] = VK_SHADER_STAGE_FRAGMENT_BIT
};

maybe_error_t maybe_vulkan_pipeline_create_shader_stage(
	VkDevice device,
	const char* path,
	maybe_vulkan_shader_type type,
	const char* entry_point,
	VkPipelineShaderStageCreateInfo* stage_info,
	VkShaderModule* module
) {
	maybe_error_t result = MAYBE_ERROR_UNINITIALIZED;
	VkShaderModuleCreateInfo shader_module_create_info = { 0 };
	VkShaderModule shader_module = VK_NULL_HANDLE;
	uint8_t* shader_code_buffer = NULL;
	uint32_t shader_code_size = 0;

	if (NULL == path) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_PIPELINE_NULL_PARAM;
		goto l_cleanup;
	}

	/* Use a default entry point if needed */
	if (NULL == entry_point) {
		entry_point = DEFAULT_SHADER_ENTRY_POINT;
	}

	/* Read compiled shader binary */
	result = maybe_utils_io_read_file(path, &shader_code_buffer, &shader_code_size);
	if (IS_FAILURE(result)) {
		goto l_cleanup;
	}

	/* Create shader module */
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = shader_code_size;
	shader_module_create_info.pCode = (const uint32_t*)shader_code_buffer;
	if (vkCreateShaderModule(device, &shader_module_create_info, NULL, &shader_module) != VK_SUCCESS) {
		result = MAYBE_ERROR_GRAPHICS_VULKAN_PIPELINE_SHADER_MODULE_CREATION_FAILED;
		goto l_cleanup;
	}		

	/* Create shader pipeline stage */
	stage_info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_info->stage = g_shader_type_to_stage[type];
	stage_info->module = shader_module;
	stage_info->pName = entry_point;

	*module = shader_module;

	result = MAYBE_ERROR_SUCCESS;
l_cleanup:
	if (NULL != shader_code_buffer) {
		free(shader_code_buffer);
	}

	return result;
}
