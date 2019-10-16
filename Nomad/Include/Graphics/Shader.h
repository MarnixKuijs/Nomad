#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>
#include <cstddef>
#include <filesystem>

namespace cof
{
	struct Shader
	{
		Shader(VkDevice device, const std::vector<std::byte>& buffer);
		~Shader() = default;
		VkShaderModule Handle() { return shaderModule; }
	private:
		VkShaderModule shaderModule;
	};

	Shader LoadShader(std::filesystem::path&& shaderPath, VkDevice device);
	Shader LoadShader(const std::filesystem::path& shaderPath, VkDevice device);
}
