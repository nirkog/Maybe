#pragma once

#include <vulkan/vulkan.h>

#include "common/common.h"

typedef enum {
	MAYBE_VULKAN_SHADER_TYPE_VERTEX = 0,
	MAYBE_VULKAN_SHADER_TYPE_FRAGMENT = 1,
} maybe_vulkan_shader_type;

/*
 * @brief Create a Vulkan pipeline stage object for a given compiled shader
 *
 * @param device A Vulkan logical device
 * @param path Path to the compiled shader binary
 * @param type Shader type
 * @param entry_point Entry point of the shader. If this argument is NULL,
 * 		  			  a default entry point will be used
 * @param stage_info A pointer to a stage info struct that will be filled with the result
 * @param module A shader module that is create for the stage, should be destroyed after
 * 				 the pipeline is created
 * */
maybe_error_t maybe_vulkan_pipeline_create_shader_stage(
	VkDevice device,
	const char* path,
	maybe_vulkan_shader_type type,
	const char* entry_point,
	VkPipelineShaderStageCreateInfo* stage_info,
	VkShaderModule* module
);
